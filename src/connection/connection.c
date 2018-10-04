#include <zend_types.h>
#include "respondphp.h"
#include "connection/connection.h"

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

static void connection_write_cb(rp_write_req_t *req, int status)
{
    zval param;
    rp_connection_ext_t *resource = FETCH_RESOURCE(((rp_client_t *) req->uv_write.handle)->connection_zo, rp_connection_ext_t);
    ZVAL_LONG(&param, status);
    rp_event_emitter_emit(&resource->event_hook, ZEND_STRL("write"), 1, &param);
    rp_free(req);
}

static void connection_shutdown_cb(uv_shutdown_t* req, int status)
{
    rp_connection_ext_t *resource = FETCH_RESOURCE(((rp_client_t *) req->handle)->connection_zo, rp_connection_ext_t);
    rp_free(req);
    connection_close(resource);
}

static void connection_close_cb(uv_handle_t* handle)
{
    zval param;
    rp_connection_ext_t *resource = FETCH_RESOURCE(((rp_client_t *) handle)->connection_zo, rp_connection_ext_t);
    ZVAL_NULL(&param);
    rp_event_emitter_emit(&resource->event_hook, ZEND_STRL("close"), 1, &param);
    releaseResource(resource);
}

static void releaseResource(rp_connection_ext_t *resource)
{
    zval tmp;
    rp_event_hook_destroy(&resource->event_hook);
    ZVAL_OBJ(&tmp, &resource->zo);
    zval_ptr_dtor(&tmp);
    rp_free(resource->client);
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) rp_malloc(suggested_size);
    buf->len = suggested_size;
}

static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    zval param;
    rp_connection_ext_t *resource = FETCH_RESOURCE(((rp_client_t *) stream)->connection_zo, rp_connection_ext_t);

    if(nread > 0){
        ZVAL_STRINGL(&param, buf->base, nread);
        rp_event_emitter_emit(&resource->event_hook, ZEND_STRL("data"), 1, &param);
        ZVAL_PTR_DTOR(&param);
    }
    else{
        ZVAL_LONG(&param, nread);
        rp_event_emitter_emit(&resource->event_hook, ZEND_STRL("error"), 1, &param);
    }

    rp_free(buf->base);
}

void rp_connection_factory(rp_client_t *client, zval *connection)
{
    object_init_ex(connection, CLASS_ENTRY(respond_connection_connection));
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(connection, rp_connection_ext_t);
    resource->client = client;
    client->connection_zo = Z_OBJ_P(connection);
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
    zend_object_std_dtor(object);
}

CLASS_ENTRY_FUNCTION_D(respond_connection_connection)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_connection_connection, "Respond\\Connection\\Connection", create_respond_connection_connection_resource);
    OBJECT_HANDLER(respond_connection_connection).offset = XtOffsetOf(rp_connection_ext_t, zo);
    OBJECT_HANDLER(respond_connection_connection).clone_obj = NULL;
    OBJECT_HANDLER(respond_connection_connection).free_obj = free_respond_connection_connection_resource;
    zend_class_implements(CLASS_ENTRY(respond_connection_connection), 1, CLASS_ENTRY(respond_socket_connection_interface));
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
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
        Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}

PHP_METHOD(respond_connection_connection, getRemoteAddress)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);

    if(resource->client->stream.stream.type == UV_TCP) {
        rp_socket_connection_tcp_getRemoteAddress(&resource->client->stream.tcp, return_value);
        return;
    }

    RETURN_NULL();
}

PHP_METHOD(respond_connection_connection, getLocalAddress)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);

    if(resource->client->stream.stream.type == UV_TCP) {
        rp_socket_connection_tcp_getLocalAddress(&resource->client->stream.tcp, return_value);
        return;
    }

    RETURN_NULL();
}

PHP_METHOD(respond_connection_connection, close)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    connection_close(resource);
}

PHP_METHOD(respond_connection_connection, isReadable)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    if(uv_is_readable((const uv_stream_t *) &resource->client->stream)){
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

PHP_METHOD(respond_connection_connection, isWritable)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    if(uv_is_writable((const uv_stream_t *) &resource->client->stream)){
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

PHP_METHOD(respond_connection_connection, write)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    char *data;
    size_t data_len;
    rp_write_req_t *req;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &data, &data_len)) {
        return;
    }

    req = rp_make_write_req(data, data_len);
    if(uv_write((uv_write_t *) req, (uv_stream_t *) &resource->client->stream, &req->buf, 1, (uv_write_cb) connection_write_cb)){
        rp_free(req);
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(respond_connection_connection, end)
{
    zval *self = getThis();
    rp_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_ext_t);
    char *data = NULL;
    size_t data_len;
    rp_write_req_t *write_req;
    uv_shutdown_t *shutdown_req;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &data, &data_len)) {
        return;
    }

    if(data != NULL) {
        write_req = rp_make_write_req(data, data_len);
        if(uv_write((uv_write_t *) write_req, (uv_stream_t *) &resource->client->stream, &write_req->buf, 1, (uv_write_cb) connection_write_cb)){
            rp_free(write_req);
            RETURN_FALSE;
        }
    }

    shutdown_req = rp_malloc(sizeof(shutdown_req));

    if(uv_shutdown(shutdown_req, (uv_stream_t *) &resource->client->stream, connection_shutdown_cb)){
        rp_free(shutdown_req);
        RETURN_FALSE;
    }
    RETURN_TRUE;
}
