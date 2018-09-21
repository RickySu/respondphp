#ifndef RP_CONNECTION_CONNECTION_H
#define RP_CONNECTION_CONNECTION_H
#include "internal/event_emitter.h"

typedef struct {
    uint flag;
    rp_client_t  *client;
    event_hook_t event_hook;
    zend_object  zo;
} rp_connection_ext_t;

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
static zend_object *create_respond_connection_connection_resource(zend_class_entry *class_type);
static void free_respond_connection_connection_resource(zend_object *object);
static void releaseResource(rp_connection_ext_t *resource);

PHP_METHOD(respond_connection_connection, __construct);

TRAIT_PHP_METHOD(respond_connection_connection, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, event_emitter);

DECLARE_FUNCTION_ENTRY(respond_connection_connection) =
{
    PHP_ME(respond_connection_connection, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    TRAIT_FUNCTION_ENTRY_ME(respond_connection_connection, event_emitter)
    PHP_FE_END
};


#endif //RP_CONNECTION_CONNECTION_H
