#ifndef _RP_SERVER_ROUTINE_H
#define _RP_SERVER_ROUTINE_H

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_routine, __construct), 0)
    ZEND_ARG_CALLABLE_INFO(0, execution, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_routine, execute), 0)
    ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    rp_reactor_t *reactor;
    zval execution;
    fcall_info_t execution_fci;
    HashTable routine_executions;
    zend_object zo;
} rp_routine_ext_t;

typedef struct write_req_s{
    uv_write_t uv_write;
    uv_buf_t buf;
} write_req_t;

typedef struct {
    uv_pipe_t pipe;
    int fd;
    zval promise;
    rp_routine_ext_t *resource;
} routine_execution_t;

static zend_object *create_respond_server_routine_resource(zend_class_entry *class_type);
static void free_respond_server_routine_resource(zend_object *object);
static void client_accept_close_cb(uv_handle_t* handle);
static void close_cb(rp_client_t *client);
static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
static void connection_cb(rp_reactor_t *reactor, int status);
static void accepted_cb(zend_object *server, rp_client_t *client, char *ipc_data, size_t ipc_data_len);
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void routine_result_read_cb(routine_execution_t *routine_execution, int status, const uv_buf_t *buf);
static void routine_result_close_cb(routine_execution_t *routine_execution);
static void routine_result_write_cb(rp_write_req_t *req, int status);
static void releaseResource(rp_routine_ext_t *resource);
static void routine_execution_free(zval *hook);
static routine_execution_t *routine_execution_add(rp_routine_ext_t *resource, zval *args);
static void routine_close_cb(uv_handle_t* handle);
static void setSelfReference(rp_routine_ext_t *resource);

PHP_METHOD(respond_server_routine, __construct);
PHP_METHOD(respond_server_routine, execute);

DECLARE_FUNCTION_ENTRY(respond_server_routine) =
{
    PHP_ME(respond_server_routine, __construct, ARGINFO(respond_server_routine, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_routine, execute, ARGINFO(respond_server_routine, execute), ZEND_ACC_PUBLIC)
    PHP_FE_END
};
#endif
