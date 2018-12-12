#include "respondphp.h"
#include "server/routine.h"

DECLARE_FUNCTION_ENTRY(respond_server_routine) =
{
    PHP_ME(respond_server_routine, __construct, ARGINFO(respond_server_routine, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_routine, execute, ARGINFO(respond_server_routine, execute), ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static fcall_info_t serialize_fci = {.fcc.initialized = 0};
static fcall_info_t unserialize_fci = {.fcc.initialized = 0};
static zend_object *create_respond_server_routine_resource(zend_class_entry *class_type);
static void free_respond_server_routine_resource(zend_object *object);
static void close_cb(rp_stream_t *client);
static void accepted_cb(zend_object *server, rp_stream_t *client, char *ipc_data, size_t ipc_data_len);
static void routine_result_read_cb(routine_execution_t *routine_execution, int status, const uv_buf_t *buf);
static void routine_result_close_cb(routine_execution_t *routine_execution);
static void routine_result_write_cb(rp_write_req_t *req, int status);
static void releaseResource(rp_routine_ext_t *resource);
static void routine_execution_free(zval *hook);
static void routine_execution_add(routine_execution_t *routine_execution, zval *args);
static void rp_serialize(zval *param, zval *serialized);
static void rp_unserialize(zval *param, char *serialized, size_t serialized_len);
static void routine_result_shutdown_cb(uv_shutdown_t* req, int status);
static void routine_async_cb(routine_execution_t *routine_execution);
static void routine_async_cb_worker_free(routine_execution_t *routine_execution);

CLASS_ENTRY_FUNCTION_D(respond_server_routine)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_routine, "Respond\\Server\\Routine", create_respond_server_routine_resource);
    OBJECT_HANDLER(respond_server_routine).offset = XtOffsetOf(rp_routine_ext_t, zo);
    OBJECT_HANDLER(respond_server_routine).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_routine).free_obj = free_respond_server_routine_resource;
}

static void releaseResource(rp_routine_ext_t *resource)
{
    zend_hash_destroy(&resource->routine_executions);
    ZVAL_PTR_DTOR(&resource->execution);
}

static void routine_execution_free(zval *item)
{
    routine_execution_t *routine_execution = Z_PTR_P(item);
    ZVAL_PTR_DTOR(&routine_execution->promise);
    rp_free(routine_execution);
}

static void routine_result_close_cb(routine_execution_t *routine_execution)
{
    rp_routine_ext_t *resource = routine_execution->resource;
    zend_hash_index_del(&resource->routine_executions, routine_execution->index);
}

static void routine_result_read_cb(routine_execution_t *routine_execution, int status, const uv_buf_t *buf)
{
    zval result;

    if(status > 0){
        rp_unserialize(&result, buf->base, status);
        rp_resolve_promise(&routine_execution->promise, &result);
        ZVAL_PTR_DTOR(&result);
    }

    rp_free(buf->base);
    uv_close((uv_handle_t *) &routine_execution->pipe, (uv_close_cb) routine_result_close_cb);
}

static void routine_execution_add(routine_execution_t *routine_execution, zval *args)
{
    int fd[2];
    char *msg = NULL;
    size_t msg_len = 0;
    uv_pipe_t *pipe;
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);

    if(!Z_ISNULL_P(args)){
        RP_ASSERT(Z_TYPE_P(args) == IS_STRING);
        msg = Z_STRVAL_P(args);
        msg_len = Z_STRLEN_P(args);
    }

    uv_pipe_init(&main_loop, &routine_execution->pipe, 0);
    uv_pipe_open(&routine_execution->pipe, fd[1]);
    routine_execution->index = zend_hash_next_free_element(&routine_execution->resource->routine_executions);
    zend_hash_index_add_ptr(&routine_execution->resource->routine_executions, routine_execution->index, routine_execution);
    uv_read_start((uv_stream_t *) &routine_execution->pipe, rp_alloc_buffer, (uv_read_cb) routine_result_read_cb);

    pipe = rp_malloc(sizeof(uv_pipe_t));
    uv_pipe_init(&main_loop, pipe, 0);
    uv_pipe_open(pipe, fd[0]);
    rp_reactor_ipc_send_ex(routine_execution->resource->reactor, (uv_stream_t *) pipe, rp_free_cb, msg, msg_len, (uv_stream_t *) &routine_pipe);
}

static zend_object *create_respond_server_routine_resource(zend_class_entry *ce)
{
    rp_routine_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_routine_ext_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_routine);
    zend_hash_init(&resource->routine_executions, 5, NULL, routine_execution_free, 0);
    return &resource->zo;
}

static void free_respond_server_routine_resource(zend_object *object)
{
    rp_routine_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_routine_ext_t);
    releaseResource(resource);
    zend_object_std_dtor(object);
}

PHP_METHOD(respond_server_routine, __construct)
{
    zval *self = getThis();
    zval *execution;
    rp_reactor_t *reactor;
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &execution)) {
        return;
    }

    reactor = rp_reactors_add_new(self);
    reactor->type = RP_ROUTINE;
    reactor->cb.stream.accepted = accepted_cb;
    resource->reactor = reactor;
    ZVAL_COPY_VALUE(&resource->execution, execution);
    zval_add_ref(&resource->execution);
}

PHP_METHOD(respond_server_routine, execute)
{
    zval *self = getThis();
    zval *args = NULL;
    zval serialized;
    routine_execution_t *routine_execution;
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &args)) {
        return;
    }


    routine_execution = rp_malloc(sizeof(routine_execution_t));
    routine_execution->resource = resource;
    rp_make_promise_object(&routine_execution->promise);
    RETVAL_ZVAL(&routine_execution->promise, 1, 0);

    if(args){
        rp_serialize(&routine_execution->args, args);
    }
    else {
        ZVAL_NULL(&routine_execution->args);
    }
    rp_reactor_async_init_ex((rp_reactor_async_cb) routine_async_cb, (rp_reactor_async_cb) routine_async_cb_worker_free, routine_execution);
}

static void routine_async_cb(routine_execution_t *routine_execution)
{
    routine_execution_add(routine_execution, &routine_execution->args);
    ZVAL_PTR_DTOR(&routine_execution->args);
}

static void routine_async_cb_worker_free(routine_execution_t *routine_execution)
{
    ZVAL_PTR_DTOR(&routine_execution->args);
    ZVAL_PTR_DTOR(&routine_execution->promise);
    rp_free(routine_execution);
}


static void close_cb(rp_stream_t *client)
{
    rp_free(client);
}

static void routine_result_shutdown_cb(uv_shutdown_t* req, int status)
{
    rp_free(req);
    uv_close((uv_handle_t *) req->handle, (uv_close_cb) close_cb);
}

static void routine_result_write_cb(rp_write_req_t *req, int status)
{
    rp_free(req);
}

static void accepted_cb(zend_object *server, rp_stream_t *client, char *ipc_data, size_t ipc_data_len)
{
    zval param, retval, serialized;
    rp_write_req_t *req;
    uv_shutdown_t *shutdown_req;
    rp_routine_ext_t *resource = FETCH_RESOURCE(server, rp_routine_ext_t);

    if(!resource->execution_fci.fcc.initialized) {
        zend_fcall_info_init(&resource->execution, 0, FCI_PARSE_PARAMETERS_CC(resource->execution_fci), NULL, NULL);
    }

    ZVAL_NULL(&param);

    if(ipc_data_len) {
        rp_unserialize(&param, ipc_data, ipc_data_len);
    }

    int ret = fci_call_function(&resource->execution_fci, &retval, 1, &param);
    rp_serialize(&serialized, &retval);

    RP_ASSERT(Z_TYPE(serialized) == IS_STRING);

    req = rp_make_write_req(Z_STRVAL(serialized), Z_STRLEN(serialized));
    uv_write((uv_write_t *) req, (uv_stream_t *) client, &req->buf, 1, (uv_write_cb) routine_result_write_cb);

    shutdown_req = rp_malloc(sizeof(uv_shutdown_t));
    uv_shutdown(shutdown_req, (uv_stream_t *) client, (uv_shutdown_cb) routine_result_shutdown_cb);

    ZVAL_PTR_DTOR(&param);
    ZVAL_PTR_DTOR(&retval);
    ZVAL_PTR_DTOR(&serialized);
}

static void rp_serialize(zval *serialized, zval *param)
{
    zval fn;

    if(!serialize_fci.fcc.initialized) {
        ZVAL_STRING(&fn, "serialize");
        zend_fcall_info_init(&fn, 0, FCI_PARSE_PARAMETERS_CC(serialize_fci), NULL, NULL);
        ZVAL_PTR_DTOR(&fn);
    }

    fci_call_function(&serialize_fci, serialized, 1, param);
}

static void rp_unserialize(zval *retval, char *serialized, size_t serialized_len)
{
    zval fn, param;

    if(!unserialize_fci.fcc.initialized) {
        ZVAL_STRING(&fn, "unserialize");
        zend_fcall_info_init(&fn, 0, FCI_PARSE_PARAMETERS_CC(unserialize_fci), NULL, NULL);
        ZVAL_PTR_DTOR(&fn);
    }

    ZVAL_STRINGL(&param, serialized, serialized_len);
    fci_call_function(&unserialize_fci, retval, 1, &param);
    ZVAL_PTR_DTOR(&param);
}
