#include <zend_types.h>
#include "respondphp.h"

#ifdef HAVE_OPENSSL
#include "stream/connection.h"
#include "stream/secure.h"

DECLARE_FUNCTION_ENTRY(respond_stream_secure) =
{
    PHP_ME(respond_stream_secure, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(respond_stream_secure, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(respond_stream_secure, isReadable, ARGINFO(respond_stream_readable_stream_interface, isReadable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_stream_secure, isWritable, ARGINFO(respond_stream_writable_stream_interface, isWritable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_stream_secure, write, ARGINFO(respond_stream_writable_stream_interface, write), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_stream_secure, end, ARGINFO(respond_stream_writable_stream_interface, end), ZEND_ACC_PUBLIC) \
    TRAIT_FUNCTION_ENTRY_ME(respond_stream_secure, event_emitter)
    TRAIT_FUNCTION_ENTRY_ME(respond_stream_secure, stream_connection)
    PHP_FE_END
};

static zend_object *create_respond_stream_secure_resource(zend_class_entry *class_type);
static void free_respond_stream_secure_resource(zend_object *object);
static void releaseResource(rp_stream_secure_ext_t *resource);
static zend_bool write_bio_to_socket(rp_stream_secure_ext_t *resource);
static void connection_close_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource);
static void connection_read_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource);
static void connection_write_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource);
static zend_bool connection_write(rp_stream_secure_ext_t *resource, void *data, size_t data_len);
static zend_bool connection_shutdown(rp_stream_secure_ext_t *resource);
static zend_bool connection_close(rp_stream_secure_ext_t *resource);

static zend_bool connection_close(rp_stream_secure_ext_t *resource)
{
    if(!(resource->connection->connection_methods.close(resource->connection))){
        return 0;
    }
    return 1;
}

static zend_bool connection_shutdown(rp_stream_secure_ext_t *resource)
{
    if(!resource->connection->connection_methods.shutdown(resource->connection)){
        return 0;
    }
    fprintf(stderr, "sec shutdown return\n");
    return 1;
}

static zend_bool connection_write(rp_stream_secure_ext_t *resource, void *data, size_t data_len)
{
    if(SSL_write(resource->ssl, data, data_len) > 0) {
        write_bio_to_socket(resource);
        return 1;
    }

    return 0;
}

static void releaseResource(rp_stream_secure_ext_t *resource)
{
    fprintf(stderr, "ssl %p\n", resource->ssl);
    SSL_free(resource->ssl);
    fprintf(stderr, "ssl free\n");

    if(resource->ctx) {
        fprintf(stderr, "ctx free\n");
        SSL_CTX_free(resource->ctx);
    }

    fprintf(stderr, "all free\n");
    zend_object_ptr_dtor(&resource->connection->zo);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_ptr_dtor(&resource->zo);
}

static void connection_close_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource)
{
    zval secure_param;
    fprintf(stderr, "stream closed\n");
    ZVAL_OBJ(&secure_param, &connection_secure_resource->zo);
    rp_event_emitter_emit_internal(&connection_secure_resource->event_hook, ZEND_STRL("close"), 1, &secure_param);
    releaseResource(connection_secure_resource);
}

static void connection_write_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource)
{
    zval secure_param[2];
    fprintf(stderr, "stream write end %d\n", n_param);
    ZVAL_OBJ(&secure_param[0], &connection_secure_resource->zo);
    ZVAL_COPY_VALUE(&secure_param[1], &param[1]);
    rp_event_emitter_emit_internal(&connection_secure_resource->event_hook, ZEND_STRL("write"), 2, secure_param);
}

static void connection_read_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource)
{
    int n_read;
    zend_string *buffer;
    zval data[2];
    rp_stream_connection_ext_t *connection_connection_resource = FETCH_OBJECT_RESOURCE(&param[0], rp_stream_connection_ext_t);

    fprintf(stderr, "sec stream connection_read_cb %d\n", Z_STRLEN(param[1]));

    BIO_write(connection_secure_resource->read_bio, Z_STRVAL(param[1]), Z_STRLEN(param[1]));

    if(connection_secure_resource->handshake){
        connection_secure_resource->handshake(n_param, param, connection_secure_resource);

        if(!SSL_is_init_finished(connection_secure_resource->ssl)){
            return;
        }

        fprintf(stderr, "ssl init ok\n");
    }

    ZVAL_OBJ(&data[0], &connection_secure_resource->zo);

    while(1) {
        buffer = zend_string_alloc(256, 0);
        n_read = SSL_read(connection_secure_resource->ssl, buffer->val, 255);
        fprintf(stderr, "read data: %d\n", n_read);

        if(n_read <= 0) {
            zend_string_release(buffer);
            break;
        }

        buffer->len = n_read;
        buffer->val[n_read] = '\0';
        ZVAL_NEW_STR(&data[1], buffer);
        fprintf(stderr, "read: %.*s\n", Z_STRLEN(data[1]), Z_STRVAL(data[1]));
        rp_event_emitter_emit_internal(&connection_secure_resource->event_hook, ZEND_STRL("data"), 2, data);
        zend_string_release(buffer);
    }
}

void rp_stream_secure_factory(SSL *ssl, zval *connection_connection, zval *connection_secure)
{
    rp_stream_connection_ext_t *connection_resource = FETCH_OBJECT_RESOURCE(connection_connection, rp_stream_connection_ext_t);
    object_init_ex(connection_secure, CLASS_ENTRY(respond_stream_secure));
    rp_stream_secure_ext_t *secure_resource = FETCH_OBJECT_RESOURCE(connection_secure, rp_stream_secure_ext_t);
    RP_ASSERT(&secure_resource->connection_methods == secure_resource);
    Z_ADDREF_P(connection_connection);
    secure_resource->connection_methods.write = connection_write;
    secure_resource->connection_methods.shutdown = connection_shutdown;
    secure_resource->connection_methods.close = connection_close;
    secure_resource->connection = connection_resource;
    fprintf(stderr, "ssl: %p\n", ssl);
    secure_resource->ssl = ssl;
    secure_resource->read_bio = BIO_new(BIO_s_mem());
    secure_resource->write_bio = BIO_new(BIO_s_mem());
    SSL_set_bio(secure_resource->ssl, secure_resource->read_bio, secure_resource->write_bio);
    rp_event_emitter_on_intrenal_ex(&connection_resource->event_hook, ZEND_STRL("data"), connection_read_cb, secure_resource);
    rp_event_emitter_on_intrenal_ex(&connection_resource->event_hook, ZEND_STRL("write"), connection_write_cb, secure_resource);
    rp_event_emitter_on_intrenal_ex(&connection_resource->event_hook, ZEND_STRL("close"), connection_close_cb, secure_resource);
}

static zend_object *create_respond_stream_secure_resource(zend_class_entry *ce)
{
    rp_stream_secure_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_stream_secure_ext_t, ce);
//    fprintf(stderr,  "%p %d alloc\n", resource, getpid());
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_stream_secure);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_stream_secure_resource(zend_object *object)
{
    rp_stream_secure_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_stream_secure_ext_t);
    zend_object_std_dtor(object);
//    fprintf(stderr, "%p %d free\n", resource, getpid());
}

CLASS_ENTRY_FUNCTION_D(respond_stream_secure)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_stream_secure, "Respond\\Stream\\Secure", create_respond_stream_secure_resource);
    OBJECT_HANDLER(respond_stream_secure).offset = XtOffsetOf(rp_stream_secure_ext_t, zo);
    OBJECT_HANDLER(respond_stream_secure).clone_obj = NULL;
    OBJECT_HANDLER(respond_stream_secure).free_obj = free_respond_stream_secure_resource;
    zend_class_implements(CLASS_ENTRY(respond_stream_secure), 1, CLASS_ENTRY(respond_stream_connection_interface));
}

PHP_METHOD(respond_stream_secure, __construct)
{
}

PHP_METHOD(respond_stream_secure, on)
{
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }

    rp_event_emitter_on(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_stream_secure, off)
{
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }

    rp_event_emitter_off(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_stream_secure, removeListeners)
{
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    zend_string *event;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &event)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event->val, event->len);
}

PHP_METHOD(respond_stream_secure, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
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

PHP_METHOD(respond_stream_secure, emit)
{
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
            Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}

PHP_METHOD(respond_stream_secure, close)
{
    zval connection;
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    if(!resource->connection_methods.close(resource)){
        RETURN_FALSE
    }
    RETURN_TRUE;
}

PHP_METHOD(respond_stream_secure, isReadable)
{
    zval connection;
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    ZVAL_OBJ(&connection, &resource->connection->zo);
    zend_call_method_with_0_params(&connection, NULL, NULL, "isReadable", return_value);
}

PHP_METHOD(respond_stream_secure, isWritable)
{
    zval connection;
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    ZVAL_OBJ(&connection, &resource->connection->zo);
    zend_call_method_with_0_params(&connection, NULL, NULL, "isWritable", return_value);
}

PHP_METHOD(respond_stream_secure, write)
{
    zval *self = getThis();
    size_t size;
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    zend_string *data;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &data)) {
        return;
    }

    if(!resource->connection_methods.write(resource, data->val, data->len)){
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_METHOD(respond_stream_secure, end)
{
    zval *self = getThis();
    size_t size;
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    zend_string *data = NULL;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|S", &data)) {
        return;
    }

    if(data && !resource->connection_methods.write(resource, data->val, data->len)) {
        RETURN_FALSE;
    }
fprintf(stderr, "start run shutdown %p %p\n", resource->connection_methods.shutdown, connection_shutdown);
    if(!resource->connection_methods.shutdown(resource)){
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_METHOD(respond_stream_secure, getRemoteAddress)
{
    zval connection;
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    ZVAL_OBJ(&connection, &resource->connection->zo);
    zend_call_method_with_0_params(&connection, NULL, NULL, "getRemoteAddress", return_value);
}

PHP_METHOD(respond_stream_secure, getLocalAddress)
{
    zval connection;
    zval *self = getThis();
    rp_stream_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_stream_secure_ext_t);
    ZVAL_OBJ(&connection, &resource->connection->zo);
    zend_call_method_with_0_params(&connection, NULL, NULL, "getLocalAddress", return_value);
}
#endif
