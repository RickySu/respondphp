#include <zend_types.h>
#include "respondphp.h"
#include "stream/connection.h"

DECLARE_FUNCTION_ENTRY(respond_stream_connection) =
{
    PHP_ME(respond_stream_connection, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(respond_stream_connection, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(respond_stream_connection, isReadable, ARGINFO(respond_stream_readable_stream_interface, isReadable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_stream_connection, isWritable, ARGINFO(respond_stream_writable_stream_interface, isWritable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_stream_connection, write, ARGINFO(respond_stream_writable_stream_interface, write), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_stream_connection, end, ARGINFO(respond_stream_writable_stream_interface, end), ZEND_ACC_PUBLIC) \
    TRAIT_FUNCTION_ENTRY_ME(respond_stream_connection, event_emitter)
    TRAIT_FUNCTION_ENTRY_ME(respond_stream_connection, stream_connection)
    PHP_FE_END
};

static void connection_write_cb(rp_write_req_t *req, int status);
static void connection_close_cb(uv_handle_t* handle);
static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
static zend_object *create_respond_stream_connection_resource(zend_class_entry *class_type);
static void free_respond_stream_connection_resource(zend_object *object);
static void releaseResource(rp_stream_connection_ext_t *resource);
static void connection_shutdown_cb(uv_shutdown_t* req, int status);
static zend_bool connection_close(rp_stream_connection_ext_t *resource);
static zend_bool connection_write(rp_stream_connection_ext_t *resource, void *data, size_t data_len);
static zend_bool connection_shutdown(rp_stream_connection_ext_t *resource);

static zend_bool connection_close(rp_stream_connection_ext_t *resource)
{
    if(resource->flag & RP_CONNECTION_CLOSED){
        return 0;
    }

    resource->flag |= RP_CONNECTION_CLOSED;
//    fprintf(stderr, "%p %d close\n", resource, getpid());
    uv_close((uv_handle_t *) resource->stream, connection_close_cb);
//    fprintf(stderr, "%p %d close end\n", resource, getpid());
    return 1;
}

static void connection_write_cb(rp_write_req_t *req, int status)
{
    zval param[2];
    rp_stream_connection_ext_t *resource = FETCH_RESOURCE(((rp_stream_t *) req->req.uv_write.handle)->connection_zo, rp_stream_connection_ext_t);
    rp_free(req);
//    fprintf(stderr, "write cb: %d %p\n", getpid(), resource);
    ZVAL_OBJ(&param[0], &resource->zo);
    ZVAL_LONG(&param[1], status);
    rp_event_emitter_emit_internal(&resource->event_hook, ZEND_STRL("write"), 2, param);

    if(resource->close_write_req == req) {
        connection_close(resource);
    }
}

static void connection_shutdown_cb(uv_shutdown_t* req, int status)
{
    rp_stream_connection_ext_t *resource = FETCH_RESOURCE(((rp_stream_t *) req->handle)->connection_zo, rp_stream_connection_ext_t);
    rp_free(req);
    connection_close(resource);
}

static void connection_close_cb(uv_handle_t* handle)
{
    zval param;
    rp_stream_connection_ext_t *resource = FETCH_RESOURCE(((rp_stream_t *) handle)->connection_zo, rp_stream_connection_ext_t);
//    fprintf(stderr, "%p %d close cb\n", resource, getpid());
    ZVAL_OBJ(&param, &resource->zo);
    rp_event_emitter_emit_internal(&resource->event_hook, ZEND_STRL("close"), 1, &param);
    releaseResource(resource);
}

static void releaseResource(rp_stream_connection_ext_t *resource)
{
    rp_free(resource->stream);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_ptr_dtor(&resource->zo);
}

static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    zval param[2];
    rp_stream_connection_ext_t *resource = FETCH_RESOURCE(((rp_stream_t *) stream)->connection_zo, rp_stream_connection_ext_t);
    zend_string *string = (zend_string *)(buf->base - XtOffsetOf(zend_string, val));
    ZVAL_OBJ(&param[0], &resource->zo);
//    fprintf(stderr, "%p %d %d recv\n", resource, getpid(), nread);
    if(nread > 0){
        string->len = nread;
        string->val[nread] = '\0';
        ZVAL_NEW_STR(&param[1], string);
        rp_event_emitter_emit_internal(&resource->event_hook, ZEND_STRL("data"), 2, param);
    }
    else{
        ZVAL_LONG(&param[1], nread);
        rp_event_emitter_emit_internal(&resource->event_hook, ZEND_STRL("error"), 2, param);
        connection_close(resource);
    }

    zend_string_release(string);
//    fprintf(stderr, "%p %d %p buffer free\n", resource, getpid(), buf->base);
}

zend_bool connection_shutdown(rp_stream_connection_ext_t *resource)
{
    uv_shutdown_t *shutdown_req;

    if(resource->flag & RP_CONNECTION_SHUTDOWN){
        return 0;
    }

    resource->flag |= RP_CONNECTION_SHUTDOWN;
    shutdown_req = rp_malloc(sizeof(uv_shutdown_t));
    return uv_shutdown(shutdown_req, (uv_stream_t *) &resource->stream->stream, (uv_shutdown_cb) connection_shutdown_cb) == 0;
}

zend_bool connection_write(rp_stream_connection_ext_t *resource, void *data, size_t data_len)
{
    rp_write_req_t *req;

    if(resource->flag & RP_CONNECTION_SHUTDOWN){
        return 0;
    }

    req = rp_make_write_req(data, data_len);
    if(uv_write((uv_write_t *) req, (uv_stream_t *) &resource->stream->stream, &req->buf, 1, (uv_write_cb) connection_write_cb)){
        rp_free(req);
        return 0;
    }
    return 1;
}

void rp_stream_connection_factory(rp_stream_t *stream, zval *connection)
{
    object_init_ex(connection, CLASS_ENTRY(respond_stream_connection));
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(connection, rp_stream_connection_ext_t);
    RP_ASSERT(&resource->connection_methods == resource);
    resource->connection_methods.write = connection_write;
    resource->connection_methods.shutdown = connection_shutdown;
    resource->connection_methods.close = connection_close;
    resource->stream = stream;
    stream->connection_zo = Z_OBJ_P(connection);
    uv_read_start((uv_stream_t*) stream, rp_alloc_buffer_zend_string, read_cb);
//    Z_ADDREF_P(stream);
    //fprintf(stderr, "init %d %p\n", getpid(), resource);
}

static zend_object *create_respond_stream_connection_resource(zend_class_entry *ce)
{
    rp_stream_connection_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_stream_connection_ext_t, ce);
//    fprintf(stderr,  "%p %d alloc\n", resource, getpid());
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_stream_connection);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_stream_connection_resource(zend_object *object)
{
    rp_stream_connection_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_stream_connection_ext_t);
    zend_object_std_dtor(object);
//    fprintf(stderr, "%p %d free\n", resource, getpid());
}

CLASS_ENTRY_FUNCTION_D(respond_stream_connection)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_stream_connection, "Respond\\Stream\\Connection", create_respond_stream_connection_resource);
    OBJECT_HANDLER(respond_stream_connection).offset = XtOffsetOf(rp_stream_connection_ext_t, zo);
    OBJECT_HANDLER(respond_stream_connection).clone_obj = NULL;
    OBJECT_HANDLER(respond_stream_connection).free_obj = free_respond_stream_connection_resource;
    zend_class_implements(CLASS_ENTRY(respond_stream_connection), 1, CLASS_ENTRY(respond_stream_connection_interface));
}

PHP_METHOD(respond_stream_connection, __construct)
{
}

PHP_METHOD(respond_stream_connection, on)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }

    rp_event_emitter_on(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_stream_connection, off)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }

    rp_event_emitter_off(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_stream_connection, removeListeners)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    zend_string *event;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &event)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event->val, event->len);
}

PHP_METHOD(respond_stream_connection, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    zend_string *event;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &event)) {
        return;
    }

    listeners = rp_event_emitter_getListeners(&resource->event_hook, event->val, event->len);

    if(listeners == NULL){
        RETURN_NULL();
    }

    RETURN_ZVAL(listeners, 1, 0);
}

PHP_METHOD(respond_stream_connection, emit)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
        Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}

PHP_METHOD(respond_stream_connection, getRemoteAddress)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);

    if(resource->stream->stream.stream.type == UV_TCP) {
        rp_stream_connection_tcp_get_remote_address(&resource->stream->stream.tcp, return_value, NULL);
        return;
    }

    RETURN_NULL();
}

PHP_METHOD(respond_stream_connection, getLocalAddress)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);

    if(resource->stream->stream.stream.type == UV_TCP) {
        rp_stream_connection_tcp_get_local_address(&resource->stream->stream.tcp, return_value, NULL);
        return;
    }

    RETURN_NULL();
}

PHP_METHOD(respond_stream_connection, close)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);

    if(resource->flag & RP_CONNECTION_SHUTDOWN){
        RETURN_FALSE;
    }

    if(!resource->connection_methods.close(resource)){
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_METHOD(respond_stream_connection, isReadable)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    if(uv_is_readable((const uv_stream_t *) &resource->stream->stream)){
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

PHP_METHOD(respond_stream_connection, isWritable)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    if(uv_is_writable((const uv_stream_t *) &resource->stream->stream)){
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

PHP_METHOD(respond_stream_connection, write)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    zend_string *data;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &data)) {
        return;
    }

    if(!resource->connection_methods.write(resource, data->val, data->len)){
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_METHOD(respond_stream_connection, end)
{
    zval *self = getThis();
    rp_stream_connection_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_connection_ext_t);
    zend_string *data = NULL;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|S", &data)) {
        return;
    }

    if(resource->flag & RP_CONNECTION_SHUTDOWN){
        RETURN_FALSE;
    }

    if(data != NULL) {
        if(!resource->connection_methods.write(resource, data->val, data->len)){
            RETURN_FALSE;
        }
    }

    resource->connection_methods.shutdown(resource);

    RETURN_TRUE;
}
