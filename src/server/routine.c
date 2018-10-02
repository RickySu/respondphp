#include "respondphp.h"
#include "server/routine.h"

static rp_reactor_t *reactor;

static void client_accept_close_cb(uv_handle_t* handle)
{
    free(handle);
}

static void setSelfReference(rp_routine_ext_t *resource)
{
}

CLASS_ENTRY_FUNCTION_D(respond_server_routine)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_routine, "Respond\\Server\\Routine", create_respond_server_routine_resource);
    OBJECT_HANDLER(respond_server_routine).offset = XtOffsetOf(rp_routine_ext_t, zo);
    OBJECT_HANDLER(respond_server_routine).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_routine).free_obj = free_respond_server_routine_resource;
    zend_class_implements(CLASS_ENTRY(respond_server_routine), 1, CLASS_ENTRY(respond_event_event_emitter_interface));
}

static void releaseResource(rp_routine_ext_t *resource)
{
}

static void routine_close_cb(uv_handle_t* handle)
{
    releaseResource((rp_routine_ext_t *) handle);
}

static void connection_cb(rp_reactor_t *reactor, int status)
{
}

static zend_object *create_respond_server_routine_resource(zend_class_entry *ce)
{
    rp_routine_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_routine_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_routine);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_server_routine_resource(zend_object *object)
{
    rp_routine_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_routine_ext_t);
    releaseResource(resource);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_std_dtor(object);
}

PHP_METHOD(respond_server_routine, __construct)
{
    zval *self = getThis();
    rp_reactor_t *reactor;
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);

    reactor = rp_reactor_add();
    reactor->type = RP_ROUTINE;
    reactor->connection_cb = connection_cb;
    reactor->accepted_cb = accepted_cb;
    reactor->server = &resource->zo;
    resource->reactor = reactor;

    fprintf(stderr, "recv accepted_cb: %p\n", reactor->accepted_cb);
}

PHP_METHOD(respond_server_routine, execute) {
    int fd[2];
    const char *msg = "123456";
    uv_pipe_t *pipe;
    zval *self = getThis();
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);
    fprintf(stderr, "execute:\n");
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    pipe = malloc(sizeof(uv_pipe_t));
    uv_pipe_init(&main_loop, pipe, 0);
    uv_pipe_open(pipe, fd[0]);
    rp_reactor_send_ex(resource->reactor, pipe, client_accept_close_cb, msg, strlen(msg), &routine_pipe);
}

static void accepted_cb(zend_object *server, rp_client_t *client, char *ipc_data, size_t ipc_data_len)
{
    rp_routine_ext_t *resource = FETCH_RESOURCE(server, rp_routine_ext_t);
    fprintf(stderr, "routine socket send %.*s\n", ipc_data_len, ipc_data);
    free(client);
}

PHP_METHOD(respond_server_routine, on)
{
    zval *self = getThis();
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }
    rp_event_emitter_on(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_server_routine, off)
{
    zval *self = getThis();
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }
    rp_event_emitter_off(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_server_routine, removeListeners)
{
    zval *self = getThis();
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);
    const char *event;
    size_t event_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &event, &event_len)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event, event_len);
}

PHP_METHOD(respond_server_routine, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);
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

PHP_METHOD(respond_server_routine, emit)
{
    zval *self = getThis();
    rp_routine_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_routine_ext_t);
    const char *event;
    size_t event_len;
    zval *params;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &params)) {
        return;
    }
    rp_event_emitter_emit(&resource->event_hook, event, event_len, params);
    RETURN_ZVAL(params, 1, 0);
}
