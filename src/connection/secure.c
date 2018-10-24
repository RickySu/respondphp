#include <zend_types.h>
#include "respondphp.h"

#ifdef HAVE_OPENSSL
#include "connection/connection.h"
#include "connection/secure.h"

DECLARE_FUNCTION_ENTRY(respond_connection_secure) =
{
    PHP_ME(respond_connection_secure, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(respond_connection_secure, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(respond_connection_secure, isReadable, ARGINFO(respond_stream_readable_stream_interface, isReadable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_secure, isWritable, ARGINFO(respond_stream_writable_stream_interface, isWritable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_secure, write, ARGINFO(respond_stream_writable_stream_interface, write), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_secure, end, ARGINFO(respond_stream_writable_stream_interface, end), ZEND_ACC_PUBLIC) \
    TRAIT_FUNCTION_ENTRY_ME(respond_connection_secure, event_emitter)
    TRAIT_FUNCTION_ENTRY_ME(respond_connection_secure, socket_connection)
    PHP_FE_END
};

static zend_object *create_respond_connection_secure_resource(zend_class_entry *class_type);
static void free_respond_connection_secure_resource(zend_object *object);
static void releaseResource(rp_connection_secure_ext_t *resource);
static int write_bio_to_socket(rp_connection_secure_ext_t *resource);

static void releaseResource(rp_connection_secure_ext_t *resource)
{
    SSL_free(resource->ssl);
    BIO_free(resource->read_bio);
    BIO_free(resource->write_bio);
    zend_object_ptr_dtor(&resource->connection->zo);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_ptr_dtor(&resource->zo);
}

static void read_cb(int n_param, zval *param, rp_connection_secure_ext_t *connection_secure_resource)
{
    int n_read;
    zend_string *buffer;
    zval data[2];
    rp_connection_connection_ext_t *connection_connection_resource = FETCH_OBJECT_RESOURCE(&param[0], rp_connection_connection_ext_t);

    fprintf(stderr, "sec connection read_cb %d\n", Z_STRLEN(param[1]));

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

void rp_connection_secure_factory(SSL *ssl, zval *connection_connection, zval *connection_secure)
{
    rp_connection_connection_ext_t *connection_resource = FETCH_OBJECT_RESOURCE(connection_connection, rp_connection_connection_ext_t);
    object_init_ex(connection_secure, CLASS_ENTRY(respond_connection_secure));
    rp_connection_secure_ext_t *secure_resource = FETCH_OBJECT_RESOURCE(connection_secure, rp_connection_secure_ext_t);
    Z_ADDREF_P(connection_connection);
    secure_resource->connection = connection_resource;
    secure_resource->ssl = ssl;
    secure_resource->read_bio = BIO_new(BIO_s_mem());
    secure_resource->write_bio = BIO_new(BIO_s_mem());
    SSL_set_bio(secure_resource->ssl, secure_resource->read_bio, secure_resource->write_bio);
    rp_event_emitter_on_intrenal_ex(&connection_resource->event_hook, ZEND_STRL("data"), read_cb, secure_resource);
}

static zend_object *create_respond_connection_secure_resource(zend_class_entry *ce)
{
    rp_connection_secure_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_connection_secure_ext_t, ce);
//    fprintf(stderr,  "%p %d alloc\n", resource, getpid());
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_connection_secure);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_connection_secure_resource(zend_object *object)
{
    rp_connection_secure_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_connection_secure_ext_t);
    zend_object_std_dtor(object);
//    fprintf(stderr, "%p %d free\n", resource, getpid());
}

CLASS_ENTRY_FUNCTION_D(respond_connection_secure)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_connection_secure, "Respond\\Connection\\Secure", create_respond_connection_secure_resource);
    OBJECT_HANDLER(respond_connection_secure).offset = XtOffsetOf(rp_connection_secure_ext_t, zo);
    OBJECT_HANDLER(respond_connection_secure).clone_obj = NULL;
    OBJECT_HANDLER(respond_connection_secure).free_obj = free_respond_connection_secure_resource;
    zend_class_implements(CLASS_ENTRY(respond_connection_secure), 1, CLASS_ENTRY(respond_socket_connection_interface));
}

PHP_METHOD(respond_connection_secure, __construct)
{
}

PHP_METHOD(respond_connection_secure, on)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }

    rp_event_emitter_on(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_connection_secure, off)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }

    rp_event_emitter_off(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_connection_secure, removeListeners)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
    const char *event;
    size_t event_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &event, &event_len)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event, event_len);
}

PHP_METHOD(respond_connection_secure, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
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

PHP_METHOD(respond_connection_secure, emit)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
            Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}

PHP_METHOD(respond_connection_secure, close)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
}

PHP_METHOD(respond_connection_secure, isReadable)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
}

PHP_METHOD(respond_connection_secure, isWritable)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
}

PHP_METHOD(respond_connection_secure, write)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
    char *data;
    size_t data_len;
    RETURN_TRUE;
}

PHP_METHOD(respond_connection_secure, end)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
    RETURN_TRUE;
}


PHP_METHOD(respond_connection_secure, getRemoteAddress)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);
    RETURN_NULL();
}

PHP_METHOD(respond_connection_secure, getLocalAddress)
{
    zval *self = getThis();
    rp_connection_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connection_secure_ext_t);

    RETURN_NULL();
}
#endif