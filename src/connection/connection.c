#include "respondphp.h"
#include "connection/connection.h"

static void releaseResource(rp_connection_ext_t *resource)
{
    rp_event_hook_destroy(&resource->event_hook);
    uv_close(&resource->client->stream, NULL);
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    zval param;
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(((rp_client_t *) stream)->connection, rp_connection_ext_t);

    if(nread > 0){
        ZVAL_STRINGL(&param, buf->base, nread);
        ZEND_ASSERT(zval_refcount_p(&param) == 1);
        rp_event_emitter_emit(&resource->event_hook, ZEND_STRL("data"), &param);
        ZVAL_PTR_DTOR(&param);
    }

    free(buf->base);
}

void rp_connection_factory(rp_client_t *client, zval *connection)
{
    object_init_ex(connection, CLASS_ENTRY(respond_connection_connection));
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(connection, rp_connection_ext_t);
    resource->client = client;
    client->connection = connection;
    fprintf(stderr, "connection: %x\n", connection);
    uv_read_start((uv_stream_t*) client, alloc_buffer, read_cb);
}

static zend_object *create_respond_connection_connection_resource(zend_class_entry *ce)
{
    rp_connection_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_connection_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_connection_connection);
    rp_event_hook_init(&resource->event_hook);
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

PHP_METHOD(respond_connection_connection, on)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }

    rp_event_emitter_on(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_connection_connection, off)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }

    rp_event_emitter_off(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_connection_connection, removeListeners)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    const char *event;
    size_t event_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &event, &event_len)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event, event_len);
}

PHP_METHOD(respond_connection_connection, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    const char *event;
    size_t event_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &event, &event_len)) {
        return;
    }

    listeners = rp_event_emitter_getListeners(&resource->event_hook, event, event_len);

    if(listeners == NULL){
        RETURN_NULL();
    }

    RETURN_ZVAL(listeners, 1, 0);
}

PHP_METHOD(respond_connection_connection, emit)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    const char *event;
    size_t event_len;
    zval *params;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &params)) {
        return;
    }

    rp_event_emitter_emit(&resource->event_hook, event, event_len, params);
    RETURN_ZVAL(params, 1, 0);
}
