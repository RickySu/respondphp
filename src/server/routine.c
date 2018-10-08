#include "respondphp.h"
#include "server/routine.h"

DECLARE_FUNCTION_ENTRY(respond_server_routine) =
{
    PHP_ME(respond_server_routine, __construct, ARGINFO(respond_server_routine, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_routine, execute, ARGINFO(respond_server_routine, execute), ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static zend_ulong routine_execution_index = 0;
static fcall_info_t serialize_fci;
static zend_bool serialize_fci_inited = 0;
static fcall_info_t unserialize_fci;
static zend_bool unserialize_fci_inited = 0;

static zend_object *create_respond_server_routine_resource(zend_class_entry *class_type);
static void free_respond_server_routine_resource(zend_object *object);
static void client_accept_close_cb(uv_handle_t* handle);
static void close_cb(rp_stream_t *client);
static void accepted_cb(zend_object *server, rp_stream_t *client, char *ipc_data, size_t ipc_data_len);
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void routine_result_read_cb(routine_execution_t *routine_execution, int status, const uv_buf_t *buf);
static void routine_result_close_cb(routine_execution_t *routine_execution);
static void routine_result_write_cb(rp_write_req_t *req, int status);
static void releaseResource(rp_routine_ext_t *resource);
static void routine_execution_free(zval *hook);
static routine_execution_t *routine_execution_add(rp_routine_ext_t *resource, zval *args);
static void rp_serialize(zval *param, zval *serialized);
static void rp_unserialize(zval *param, char *serialized, size_t serialized_len);
static void routine_result_shutdown_cb(uv_shutdown_t* req, int status);

static void client_accept_close_cb(uv_handle_t* handle)
{
//    fprintf(stderr, "%p %d reactor send free\n", handle, getpid());
    rp_free(handle);
}

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
//    fprintf(stderr, "%p %d %d routine_execution_free\n", routine_execution, getpid(), Z_REFCOUNT(routine_execution->promise));
    ZVAL_PTR_DTOR(&routine_execution->promise);
    rp_free(routine_execution);
//    fprintf(stderr, "%p %d routine_execution_free end\n", routine_execution, getpid());
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) rp_malloc(suggested_size);
    buf->len = suggested_size;
}

static void routine_result_close_cb(routine_execution_t *routine_execution)
{
    rp_routine_ext_t *resource = routine_execution->resource;

//    fprintf(stderr, "%p %d result close cb\n", routine_execution, getpid());
    zend_hash_index_del(&resource->routine_executions, routine_execution->index);
}

static void routine_result_read_cb(routine_execution_t *routine_execution, int status, const uv_buf_t *buf)
{
    zval result;

    if(status > 0){
        rp_unserialize(&result, buf->base, status);
        rp_resolve_promise(&routine_execution->promise, &result);
        ZVAL_PTR_DTOR(&result);
//        fprintf(stderr, "%p %d result read cb\n", routine_execution, getpid());
    }

    rp_free(buf->base);
    uv_close((uv_handle_t *) &routine_execution->pipe, (uv_close_cb) routine_result_close_cb);
}

static routine_execution_t *routine_execution_add(rp_routine_ext_t *resource, zval *args)
{
    int fd[2];
    char *msg = NULL;
    size_t msg_len = 0;
    uv_pipe_t *pipe;
    routine_execution_t *routine_execution;
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);

    if(args){
        RP_ASSERT(Z_TYPE_P(args) == IS_STRING);
        msg = Z_STRVAL_P(args);
        msg_len = Z_STRLEN_P(args);
    }

    routine_execution = rp_malloc(sizeof(routine_execution_t));
    routine_execution->resource = resource;
    uv_pipe_init(&main_loop, &routine_execution->pipe, 0);
    uv_pipe_open(&routine_execution->pipe, fd[1]);
    routine_execution->index = routine_execution_index++;
    zend_hash_index_add_ptr(&resource->routine_executions, routine_execution->index, routine_execution);
    uv_read_start((uv_stream_t *) &routine_execution->pipe, alloc_buffer, (uv_read_cb) routine_result_read_cb);

    pipe = rp_malloc(sizeof(uv_pipe_t));
    uv_pipe_init(&main_loop, pipe, 0);
    uv_pipe_open(pipe, fd[0]);
//    fprintf(stderr, "%p %d %p reactor send\n", pipe, getpid(), routine_execution);
    rp_reactor_send_ex(resource->reactor, (uv_stream_t *) pipe, client_accept_close_cb, msg, msg_len, (uv_stream_t *) &routine_pipe);
    return routine_execution;
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

    reactor = rp_reactor_add();
    reactor->type = RP_ROUTINE;
    reactor->accepted_cb = accepted_cb;
    reactor->server = &resource->zo;
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

    if(args){
        rp_serialize(&serialized, args);
        routine_execution = routine_execution_add(resource, &serialized);
        zval_ptr_dtor(&serialized);
    }
    else {
        routine_execution = routine_execution_add(resource, args);
    }

    rp_make_promise_object(&routine_execution->promise);
    RETVAL_ZVAL(&routine_execution->promise, 1, 0);
}

static void close_cb(rp_stream_t *client)
{
    rp_free(client);
}

static void routine_result_shutdown_cb(uv_shutdown_t* req, int status)
{
    rp_free(req);
//    fprintf(stderr, "%p %d result send shutdown cb\n", routine_execution, getpid());
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

    if(!serialize_fci_inited) {
        ZVAL_STRING(&fn, "serialize");
        zend_fcall_info_init(&fn, 0, FCI_PARSE_PARAMETERS_CC(serialize_fci), NULL, NULL);
        ZVAL_PTR_DTOR(&fn);
        serialize_fci_inited = 1;
    }

    fci_call_function(&serialize_fci, serialized, 1, param);
}

static void rp_unserialize(zval *retval, char *serialized, size_t serialized_len)
{
    zval fn, param;

    if(!unserialize_fci_inited) {
        ZVAL_STRING(&fn, "unserialize");
        zend_fcall_info_init(&fn, 0, FCI_PARSE_PARAMETERS_CC(unserialize_fci), NULL, NULL);
        ZVAL_PTR_DTOR(&fn);
        unserialize_fci_inited = 1;
    }

    ZVAL_STRINGL(&param, serialized, serialized_len);
    fci_call_function(&unserialize_fci, retval, 1, &param);
    ZVAL_PTR_DTOR(&param);
}
