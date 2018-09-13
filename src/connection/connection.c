#include "respondphp.h"
#include "connection/connection.h"

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if(nread > 0){
        fprintf(stderr, "recv(%d)\n %.*s\n", nread, nread, buf->base);
    }

    free(buf->base);
}

zval *rp_connection_factory(rp_client_t *client)
{
    zval instance;
    object_init_ex(&instance, CLASS_ENTRY(respond_connection_connection));
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(&instance, rp_connection_ext_t);
    resource->client = client;
    ZVAL_COPY_VALUE(&resource->zobject, &instance);
    uv_read_start((uv_stream_t*) client, alloc_buffer, read_cb);
    client->connection = &resource->zobject;
    return client->connection;
}

static zend_object *create_respond_connection_connection_resource(zend_class_entry *ce)
{
    rp_connection_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_connection_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_connection_connection);
    return &resource->zo;
}

static void free_respond_connection_connection_resource(zend_object *object)
{
    rp_connection_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_connection_ext_t);
    releaseResource(resource);
    zend_object_std_dtor(object);
}

CLASS_ENTRY_FUNCTION_D(respond_connection_connection)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_connection_connection, "Respond\\Connection\\Connection", create_respond_connection_connection_resource);
    OBJECT_HANDLER(respond_connection_connection).offset = XtOffsetOf(rp_connection_ext_t, zo);
    OBJECT_HANDLER(respond_connection_connection).clone_obj = NULL;
    OBJECT_HANDLER(respond_connection_connection).free_obj = free_respond_connection_connection_resource;
}

PHP_METHOD(respond_connection_connection, __construct)
{
}
