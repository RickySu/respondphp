#include "respondphp.h"
#include "server/pipe.h"

static zend_object *create_respond_server_pipe_resource(zend_class_entry *class_type);
static void free_respond_server_pipe_resource(zend_object *object);
static void client_accept_close_cb(uv_handle_t* handle);
static void connection_cb(rp_reactor_t *reactor, int status);
static void accepted_cb(zend_object *server, rp_client_t *client);
static void releaseResource(rp_pipe_ext_t *resource);

static void client_accept_close_cb(uv_handle_t* handle)
{
    rp_free(handle);
}

CLASS_ENTRY_FUNCTION_D(respond_server_pipe)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_pipe, "Respond\\Server\\Pipe", create_respond_server_pipe_resource);
    OBJECT_HANDLER(respond_server_pipe).offset = XtOffsetOf(rp_pipe_ext_t, zo);
    OBJECT_HANDLER(respond_server_pipe).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_pipe).free_obj = free_respond_server_pipe_resource;
    zend_class_implements(CLASS_ENTRY(respond_server_pipe), 1, CLASS_ENTRY(respond_event_event_emitter_interface));
}

static void releaseResource(rp_pipe_ext_t *resource)
{
    if(rp_get_task_type() == ACTOR){
        unlink(resource->reactor->addr.socket_path);
    }
    rp_free(resource->reactor->addr.socket_path);
}

static void connection_cb(rp_reactor_t *reactor, int status)
{
    if (status < 0) {
        return;
    }

    uv_pipe_t *client = (uv_pipe_t*) rp_malloc(sizeof(uv_pipe_t));
    uv_pipe_init(&main_loop, client, 0);
    
    if (uv_accept((uv_stream_t *) &reactor->handler.pipe, (uv_stream_t*) client) == 0) {
        rp_reactor_send(reactor, (uv_stream_t *) client, client_accept_close_cb);
        return;
    }
    
    uv_close((uv_handle_t *) client, client_accept_close_cb);
}

static zend_object *create_respond_server_pipe_resource(zend_class_entry *ce)
{
    rp_pipe_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_pipe_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_pipe);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_server_pipe_resource(zend_object *object)
{
    rp_pipe_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_pipe_ext_t);
    releaseResource(resource);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_std_dtor(object);
    rp_free(resource);
}

PHP_METHOD(respond_server_pipe, __construct)
{
    zval *self = getThis();
    const char *socket_path = NULL;
    size_t socket_path_len;

    rp_reactor_t *reactor;
    rp_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_pipe_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &socket_path, &socket_path_len)) {
        return;
    }

    reactor = rp_reactor_add();
    reactor->addr.socket_path = rp_calloc(1, socket_path_len + 1);
    reactor->type = RP_PIPE;
    reactor->connection_cb = (rp_connection_cb) connection_cb;
    reactor->accepted_cb = (rp_accepted_cb) accepted_cb;
    reactor->server = &resource->zo;
    resource->reactor = reactor;
    memcpy((void *) reactor->addr.socket_path, socket_path, socket_path_len);
}

static void accepted_cb(zend_object *server, rp_client_t *client)
{
    zval connection;
    rp_pipe_ext_t *resource = FETCH_RESOURCE(server, rp_pipe_ext_t);
    rp_connection_factory(client, &connection);
    RP_ASSERT(zval_refcount_p(&connection) == 1);
    rp_event_emitter_emit(&resource->event_hook, ZEND_STRL("connect"), 1, &connection);
    RP_ASSERT(zval_refcount_p(&connection) >= 1);
}

PHP_METHOD(respond_server_pipe, close)
{
//    zval *self = getThis();
//    rp_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_pipe_ext_t);
}

PHP_METHOD(respond_server_pipe, on)
{
    zval *self = getThis();
    rp_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_pipe_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }
    rp_event_emitter_on(&resource->event_hook, event, event_len, hook);
//    zend_print_zval_r(&resource->event_hook.hook, 0);
}

PHP_METHOD(respond_server_pipe, off)
{
    zval *self = getThis();
    rp_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_pipe_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }
    rp_event_emitter_off(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_server_pipe, removeListeners)
{
    zval *self = getThis();
    rp_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_pipe_ext_t);
    const char *event;
    size_t event_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &event, &event_len)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event, event_len);
}

PHP_METHOD(respond_server_pipe, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_pipe_ext_t);
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

PHP_METHOD(respond_server_pipe, emit)
{
    zval *self = getThis();
    rp_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_pipe_ext_t);
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
        Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}