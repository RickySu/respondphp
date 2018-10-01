#ifndef _RP_SERVER_TASK_H
#define _RP_SERVER_TASK_H
#include "internal/event_emitter.h"

static rp_reactor_t *reactor;

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_task, execute), 0)
    ZEND_ARG_INFO(0, job)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    rp_reactor_t *reactor;
    event_hook_t event_hook;
    zend_object zo;
} rp_task_ext_t;

typedef struct write_req_s{
    uv_write_t uv_write;
    uv_buf_t buf;
} write_req_t;

static zend_object *create_respond_server_task_resource(zend_class_entry *class_type);
static void free_respond_server_task_resource(zend_object *object);
static void client_accept_close_cb(uv_handle_t* handle);
static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
static void connection_cb(rp_reactor_t *reactor, int status);
static void accepted_cb(zend_object *server, rp_client_t *client, char *ipc_data, size_t ipc_data_len);
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void releaseResource(rp_task_ext_t *resource);
static void task_close_cb(uv_handle_t* handle);
static void setSelfReference(rp_task_ext_t *resource);

PHP_METHOD(respond_server_task, __construct);
PHP_METHOD(respond_server_task, execute);

TRAIT_PHP_METHOD(respond_server_task, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_task, event_emitter);
DECLARE_FUNCTION_ENTRY(respond_server_task) =
{
    PHP_ME(respond_server_task, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_task, execute, ARGINFO(respond_server_task, execute), ZEND_ACC_PUBLIC)
    TRAIT_FUNCTION_ENTRY_ME(respond_server_task, event_emitter)
    PHP_FE_END
};
#endif
