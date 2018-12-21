#include "respondphp.h"
#include "server/http.h"

DECLARE_FUNCTION_ENTRY(respond_server_http) =
{
    PHP_ME(respond_server_http, __construct, ARGINFO(respond_server_http, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_http, close, NULL, ZEND_ACC_PUBLIC)
    TRAIT_FUNCTION_ENTRY_ME(respond_server_http, event_emitter)
    PHP_FE_END
};

static zend_object *create_respond_server_http_resource(zend_class_entry *class_type);
static void free_respond_server_http_resource(zend_object *object);
static void accepted_cb(int n_param, zval *param, rp_server_http_ext_t *resource);
static void releaseResource(rp_server_http_ext_t *resource);
static void http_connection_read_cb(int n_param, zval *param, rp_http_connection_t *http_connection);
static int http_parse_header(zend_string **header_buffer, const char *buffer, size_t buffer_len, zval *parsed);
static void http_add_header_field(zval *parsed, const char *header, size_t header_len, const char *value, size_t value_len);

CLASS_ENTRY_FUNCTION_D(respond_server_http)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_http, "Respond\\Server\\HTTP", create_respond_server_http_resource);
    OBJECT_HANDLER(respond_server_http).offset = XtOffsetOf(rp_server_http_ext_t, zo);
    OBJECT_HANDLER(respond_server_http).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_http).free_obj = free_respond_server_http_resource;
    zend_class_implements(CLASS_ENTRY(respond_server_http), 1, CLASS_ENTRY(respond_event_event_emitter_interface));
}

static void releaseResource(rp_server_http_ext_t *resource)
{
}

static zend_object *create_respond_server_http_resource(zend_class_entry *ce)
{
    rp_server_http_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_server_http_ext_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_http);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_server_http_resource(zend_object *object)
{
    rp_server_http_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_server_http_ext_t);
    if(resource->socket_zo) {
        zend_object_ptr_dtor(resource->socket_zo);
    }
    releaseResource(resource);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_std_dtor(object);
}

PHP_METHOD(respond_server_http, __construct)
{
    zval *self = getThis();
    zval *socket, *options;
    rp_server_http_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_http_ext_t);
    event_hook_t *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "za", &socket, &options)) {
        return;
    }

    hook = (event_hook_t *) ((void *) Z_OBJ_P(socket) - sizeof(event_hook_t));
    rp_event_emitter_on_intrenal_ex(hook, ZEND_STRL("connect"), (rp_event_emitter_internal_cb) accepted_cb, resource);
    resource->socket_zo = Z_OBJ_P(socket);
    Z_ADDREF_P(socket);
}

static void accepted_cb(int n_param, zval *param, rp_server_http_ext_t *server_resource)
{
    rp_stream_connection_ext_t *connection_resource = FETCH_OBJECT_RESOURCE(&param[0], rp_stream_connection_ext_t);
    rp_http_connection_t *http_connection = ecalloc(1, sizeof(rp_http_connection_t));
    http_connection->connection_resource = connection_resource;
    http_connection->server_resource = server_resource;
    rp_event_emitter_on_intrenal_ex(&connection_resource->event_hook, ZEND_STRL("data"), http_connection_read_cb, http_connection);
//    rp_event_emitter_on_intrenal_ex(&connection_resource->event_hook, ZEND_STRL("write"), connection_write_cb, http_connection);
//    rp_event_emitter_on_intrenal_ex(&connection_resource->event_hook, ZEND_STRL("close"), connection_close_cb, http_connection);
}

static int http_parse_header(zend_string **header_buffer, const char *buffer, size_t buffer_len, zval *parsed)
{
    int status;
    int minor_version;
    const char *method;
    size_t method_len;
    struct phr_header headers[64];
    size_t num_headers = sizeof(headers) / sizeof(headers[0]);
    const char *path;
    size_t path_len;
    const char protocol[16];
    size_t protocol_len;
    zval server;

    if(*header_buffer == NULL){
        *header_buffer = rp_init_empty_zend_string(RP_HTTP_HEADER_MAX);
    }

    if((*header_buffer)->len + buffer_len > RP_HTTP_HEADER_MAX){
        goto header_parse_error;
    }

    memcpy(&(*header_buffer)->val[(*header_buffer)->len], buffer, buffer_len);
    (*header_buffer)->len += buffer_len;

    status = phr_parse_request((*header_buffer)->val, (*header_buffer)->len, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, 0);

    if(status == -1) {
        // error
        goto header_parse_error;
    }

    if(status == -2) {
        // request is partial
        status = 0;
        goto header_parse_error;
    }

    protocol_len = snprintf(protocol, sizeof(protocol), "HTTP/1.%d", minor_version);
    array_init(parsed);
    array_init(&server);
    add_assoc_zval(parsed, "SERVER", &server);
    add_assoc_stringl(&server, "SERVER_PROTOCOL", protocol, protocol_len);
    add_assoc_stringl(&server, "REQUEST_METHOD", method, method_len);
    add_assoc_stringl(&server, "REQUEST_URI", path, path_len);
    for(int i = 0; i< num_headers; i++) {
        http_add_header_field(&server, headers[i].name, headers[i].name_len, headers[i].value, headers[i].value_len);
    }

    header_parse_error:
        zend_string_release(*header_buffer);
        *header_buffer = NULL;
        return status;
}

static void http_add_header_field(zval *parsed, const char *header, size_t header_len, const char *value, size_t value_len)
{
    size_t header_buffer_len = header_len + sizeof("HTTP_");
    char header_buffer[header_buffer_len + 1];
    sprintf(header_buffer, "HTTP_%.*s", header_len, header);
    for(int i = 0; i < header_buffer_len; i++ ){
        if(header_buffer[i] == '-'){
            header_buffer[i] = '_';
        }
    }
    php_strtoupper(header_buffer, header_buffer_len);
    add_assoc_stringl_ex(parsed, header_buffer, header_buffer_len, value, value_len);
}

static void http_connection_read_cb(int n_param, zval *param, rp_http_connection_t *http_connection)
{
    zval parsed;
    switch(http_connection->flag) {
        case RP_HTTP_PARSE_HEADER:
            http_parse_header(&http_connection->buffer, Z_STRVAL(param[1]), Z_STRLEN(param[1]), &parsed);
            zend_print_zval_r(&parsed, 0);
            ZVAL_PTR_DTOR(&parsed);
            break;
        default:
            break;
    }

}

PHP_METHOD(respond_server_http, close)
{
//    zval *self = getThis();
//    rp_server_http_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_http_ext_t);
}

PHP_METHOD(respond_server_http, on)
{
    zval *self = getThis();
    rp_server_http_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_http_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }
    rp_event_emitter_on(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_server_http, off)
{
    zval *self = getThis();
    rp_server_http_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_http_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }
    rp_event_emitter_off(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_server_http, removeListeners)
{
    zval *self = getThis();
    rp_server_http_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_http_ext_t);
    zend_string *event;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &event)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event->val, event->len);
}

PHP_METHOD(respond_server_http, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_server_http_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_http_ext_t);
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

PHP_METHOD(respond_server_http, emit)
{
    zval *self = getThis();
    rp_server_http_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_http_ext_t);
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
        Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}
