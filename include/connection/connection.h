#ifndef RP_CONNECTION_CONNECTION_H
#define RP_CONNECTION_CONNECTION_H
#include "internal/event_emitter.h"
#include "internal/socket_connection.h"
#include "interface/stream_readable_stream_interface.h"
#include "interface/stream_writable_stream_interface.h"

typedef struct {
    uint flag;
    rp_client_t  *client;
    event_hook_t event_hook;
    zend_object  zo;
} rp_connection_ext_t;

static void connection_shutdown_cb(uv_shutdown_t* req, int status);
static void connection_write_cb(rp_write_req_t *req, int status);
static void connection_close_cb(uv_handle_t* handle);
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
static zend_object *create_respond_connection_connection_resource(zend_class_entry *class_type);
static void free_respond_connection_connection_resource(zend_object *object);
static void releaseResource(rp_connection_ext_t *resource);

static zend_always_inline void connection_close(rp_connection_ext_t *resource)
{
    if(resource->flag & RP_CONNECTION_CLOSED){
        return;
    }

    resource->flag |= RP_CONNECTION_CLOSED;
    uv_close((uv_handle_t *) resource->client, connection_close_cb);
}

PHP_METHOD(respond_connection_connection, __construct);
PHP_METHOD(respond_connection_connection, isReadable);
PHP_METHOD(respond_connection_connection, isWritable);
PHP_METHOD(respond_connection_connection, close);
PHP_METHOD(respond_connection_connection, write);
PHP_METHOD(respond_connection_connection, end);

TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, stream_readable_stream);
TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, stream_writable_stream);

TRAIT_PHP_METHOD(respond_connection_connection, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, event_emitter);

TRAIT_PHP_METHOD(respond_connection_connection, socket_connection);
TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, socket_connection);

DECLARE_FUNCTION_ENTRY(respond_connection_connection) =
{
    PHP_ME(respond_connection_connection, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(respond_connection_connection, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(respond_connection_connection, isReadable, ARGINFO(respond_stream_readable_stream_interface, isReadable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_connection, isWritable, ARGINFO(respond_stream_writable_stream_interface, isWritable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_connection, write, ARGINFO(respond_stream_writable_stream_interface, write), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_connection, end, ARGINFO(respond_stream_writable_stream_interface, end), ZEND_ACC_PUBLIC) \
    TRAIT_FUNCTION_ENTRY_ME(respond_connection_connection, event_emitter)
    TRAIT_FUNCTION_ENTRY_ME(respond_connection_connection, socket_connection)
    PHP_FE_END
};

#endif //RP_CONNECTION_CONNECTION_H
