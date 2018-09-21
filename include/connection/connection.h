#ifndef RP_CONNECTION_CONNECTION_H
#define RP_CONNECTION_CONNECTION_H

typedef struct {
    uint flag;
    rp_client_t  *client;
    zend_object  zo;
} rp_connection_ext_t;

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
static zend_object *create_respond_connection_connection_resource(zend_class_entry *class_type);
static void free_respond_connection_connection_resource(zend_object *object);
static void releaseResource(rp_connection_ext_t *resource);

PHP_METHOD(respond_connection_connection, __construct);
DECLARE_FUNCTION_ENTRY(respond_connection_connection) =
{
    PHP_ME(respond_connection_connection, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_FE_END
};
#endif //RP_CONNECTION_CONNECTION_H
