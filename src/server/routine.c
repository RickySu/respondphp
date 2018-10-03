#include "respondphp.h"
#include "server/routine.h"

static void client_accept_close_cb(uv_handle_t* handle)
{
    rp_free(handle);
}

static void setSelfReference(rp_routine_ext_t *resource)
{
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

static void routine_close_cb(uv_handle_t* handle)
{
}

static void connection_cb(rp_reactor_t *reactor, int status)
{
}

static void routine_execution_free(zval *hook)
{
    fprintf(stderr, "ht free: %p\n", Z_PTR_P(hook));
    rp_free(Z_PTR_P(hook));
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) rp_malloc(suggested_size);
    buf->len = suggested_size;
}

static void routine_result_close_cb(routine_execution_t *routine_execution)
{
    fprintf(stderr, "rraaaaa\n");
    rp_routine_ext_t *resource = routine_execution->resource;
    fprintf(stderr, "ht del: %d, %d\n", getpid(), routine_execution->fd);
    zend_hash_index_del(&resource->routine_executions, routine_execution->fd);
}

static void routine_result_read_cb(routine_execution_t *routine_execution, int status, const uv_buf_t *buf)
{
    zval result;
    fprintf(stderr, "read result: %p\n", routine_execution);
    rp_routine_ext_t *resource = routine_execution->resource;
    if(status > 0){
        fprintf(stderr, "read %.*s\n", status, buf->base);
        rp_unserialize(&result, buf->base, status);
        zend_print_zval_r(&result, 0);
        ZVAL_PTR_DTOR(&result);
    }
    rp_free(buf->base);
    uv_close(&routine_execution->pipe, (uv_close_cb) routine_result_close_cb);
}

static routine_execution_t *routine_execution_add(rp_routine_ext_t *resource, zval *args)
{
    int fd[2];
    char *msg = NULL;
    size_t msg_len = 0;
    uv_pipe_t *pipe;
    routine_execution_t *routine_execution;
    fprintf(stderr, "execute:\n");
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    pipe = rp_malloc(sizeof(uv_pipe_t));
    uv_pipe_init(&main_loop, pipe, 0);
    uv_pipe_open(pipe, fd[0]);

    if(args){
        RP_ASSERT(Z_TYPE_P(args) == IS_STRING);
        msg = Z_STRVAL_P(args);
        msg_len = Z_STRLEN_P(args);
    }

    rp_reactor_send_ex(resource->reactor, pipe, client_accept_close_cb, msg, msg_len, &routine_pipe);
    routine_execution = rp_malloc(sizeof(routine_execution_t));
    routine_execution->resource = resource;
    uv_pipe_init(&main_loop, &routine_execution->pipe, 0);
    uv_pipe_open(&routine_execution->pipe, fd[1]);
    routine_execution->fd = fd[1];
    zend_hash_index_add_ptr(&resource->routine_executions, routine_execution->fd, routine_execution);
    uv_read_start(&routine_execution->pipe, alloc_buffer, (uv_read_cb) routine_result_read_cb);

    fprintf(stderr, "ht add: %d %d %p\n", getpid(), routine_execution->fd, routine_execution);
    return routine_execution;
}

static zend_object *create_respond_server_routine_resource(zend_class_entry *ce)
{
    rp_routine_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_routine_ext_t);
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
    reactor->connection_cb = connection_cb;
    reactor->accepted_cb = accepted_cb;
    reactor->server = &resource->zo;
    resource->reactor = reactor;
    ZVAL_COPY_VALUE(&resource->execution, execution);
    zval_add_ref(&resource->execution);
    fprintf(stderr, "recv accepted_cb: %p\n", reactor->accepted_cb);
}

PHP_METHOD(respond_server_routine, execute)
{
    zval *self = getThis();
    zval *args = NULL;
    zval serialized;
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &args)) {
        return;
    }

    if(args){
        rp_serialize(&serialized, args);
        routine_execution_add(resource, &serialized);
        zval_ptr_dtor(&serialized);
        return;
    }

    routine_execution_add(resource, args);
}

static void close_cb(rp_client_t *client)
{
    rp_free(client);
}

static void accepted_cb(zend_object *server, rp_client_t *client, char *ipc_data, size_t ipc_data_len)
{
    zval param, retval, serialized;
    rp_write_req_t *req;
    rp_routine_ext_t *resource = FETCH_RESOURCE(server, rp_routine_ext_t);
    fprintf(stderr, "routine socket send %.*s\n", ipc_data_len, ipc_data);

    if(!resource->execution_fci.fcc.initialized) {
        fprintf(stderr, "init fci: %d\n", getpid());
        zend_fcall_info_init(&resource->execution, 0, FCI_PARSE_PARAMETERS_CC(resource->execution_fci), NULL, NULL);
    }

    ZVAL_NULL(&param);

    if(ipc_data_len) {
        rp_unserialize(&param, ipc_data, ipc_data_len);
    }

    int ret = fci_call_function(&resource->execution_fci, &retval, 1, &param);
    fprintf(stderr, "result: %p %d %d\n", client, getpid(), ret);
    fprintf(stderr, "routine socket end %.*s\n", ipc_data_len, ipc_data);
    rp_serialize(&serialized, &retval);

    RP_ASSERT(Z_TYPE(serialized) == IS_STRING);

    req = rp_make_write_req(Z_STRVAL(serialized), Z_STRLEN(serialized));
    uv_write(req, (uv_stream_t *) client, &req->buf, 1, routine_result_write_cb);
    ZVAL_PTR_DTOR(&param);
    ZVAL_PTR_DTOR(&retval);
    ZVAL_PTR_DTOR(&serialized);
}

static void routine_result_write_cb(rp_write_req_t *req, int status)
{
    fprintf(stderr, "result write end: %p\n", req->uv_write.handle);
    uv_close((uv_handle_t *) req->uv_write.handle, (uv_close_cb) close_cb);
    rp_free(req);
}

static void rp_serialize(zval *serialized, zval *param)
{
    fcall_info_t fci;
    zval fn;
    ZVAL_STRING(&fn, "serialize");
    zend_fcall_info_init(&fn, 0, FCI_PARSE_PARAMETERS_CC(fci), NULL, NULL);
    ZVAL_PTR_DTOR(&fn);
    fci_call_function(&fci, serialized, 1, param);
}


static void rp_unserialize(zval *retval, char *serialized, size_t serialized_len)
{
    fcall_info_t fci;
    zval fn, param;
    ZVAL_STRING(&fn, "unserialize");
    ZVAL_STRINGL(&param, serialized, serialized_len);
    zend_fcall_info_init(&fn, 0, FCI_PARSE_PARAMETERS_CC(fci), NULL, NULL);
    fci_call_function(&fci, retval, 1, &param);
    ZVAL_PTR_DTOR(&fn);
    ZVAL_PTR_DTOR(&param);
}
