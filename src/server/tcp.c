#include "respondphp.h"
#include "server/tcp.h"

static void client_accept_close_cb(uv_handle_t* handle)
{
    free(handle);
}

void tcp_close_socket(rp_tcp_ext_t *handle)
{
    if(handle->flag & UV_TCP_CLOSING_START) {
         return;
    }
    handle->flag |= UV_TCP_CLOSING_START;
    uv_close((uv_handle_t *) handle, tcp_close_cb);
}

void setSelfReference(rp_tcp_ext_t *resource)
{
    if(resource->flag & UV_TCP_HANDLE_INTERNAL_REF) {
        return;
    }
    resource->flag |= UV_TCP_HANDLE_INTERNAL_REF;
}

CLASS_ENTRY_FUNCTION_D(respond_server_tcp)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_tcp, "Respond\\Server\\Tcp", create_respond_server_tcp_resource);
    OBJECT_HANDLER(respond_server_tcp).offset = XtOffsetOf(rp_tcp_ext_t, zo);
    OBJECT_HANDLER(respond_server_tcp).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_tcp).free_obj = free_respond_server_tcp_resource;
    zend_class_implements(CLASS_ENTRY(respond_server_tcp), 1, CLASS_ENTRY(respond_event_event_emitter_interface));
}

static void releaseResource(rp_tcp_ext_t *resource)
{
    if(resource->flag & UV_TCP_HANDLE_INTERNAL_REF) {
        resource->flag &= ~UV_TCP_HANDLE_INTERNAL_REF;
    }
}

static void tcp_close_cb(uv_handle_t* handle)
{
    releaseResource((rp_tcp_ext_t *) handle);
}

static void connection_cb(rp_reactor_t *reactor, int status)
{
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(&main_loop, client);
    
    if (uv_accept((uv_stream_t *) &reactor->handler.tcp, (uv_stream_t*) client) == 0) {
        fprintf(stderr, "client close cb: %x\n", client_accept_close_cb);
        rp_reactor_send(reactor, client, client_accept_close_cb);
        return;
    }
    
    uv_close((uv_handle_t *) client, client_accept_close_cb);
}

static zend_object *create_respond_server_tcp_resource(zend_class_entry *ce)
{
    rp_tcp_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_tcp_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_tcp);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_server_tcp_resource(zend_object *object)
{
    rp_tcp_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_tcp_ext_t);
    releaseResource(resource);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_std_dtor(object);
}

PHP_METHOD(respond_server_tcp, __construct)
{
    long ret, port;
    zval *self = getThis();
    const char *host = NULL;
    size_t host_len;
    char cstr_host[30];
    struct sockaddr_in addr;
    rp_reactor_t *reactor;
    rp_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_tcp_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sl", &host, &host_len, &port)) {
        return;
    }
    
    if(host_len == 0 || host_len >= 30) {
        RETURN_LONG(-1);
    }
    
    memcpy(cstr_host, host, host_len);
    cstr_host[host_len] = '\0';
    
    if((ret = uv_ip4_addr(cstr_host, port&0xffff, &addr)) != 0) {
        RETURN_LONG(ret);
    }

    reactor = rp_reactor_add();
    reactor->type = RP_TCP;
    memcpy(&reactor->addr.socket_addr, &addr, sizeof(addr));
    reactor->connection_cb = connection_cb;
    reactor->accepted_cb = accepted_cb;
    reactor->server = &resource->zo;
    RETURN_LONG(ret);
}

static void accepted_cb(zend_object *server, rp_client_t *client)
{
    zval connection;
    rp_tcp_ext_t *resource = FETCH_RESOURCE(server, rp_tcp_ext_t);
    rp_connection_factory(client, &connection);
    ZEND_ASSERT(zval_refcount_p(&connection) == 1);
    rp_event_emitter_emit(&resource->event_hook, ZEND_STRL("connect"), &connection);
    ZEND_ASSERT(zval_refcount_p(&connection) >= 1);
}

PHP_METHOD(respond_server_tcp, close)
{
    long ret;
    zval *self = getThis();
    rp_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_tcp_ext_t);
//    tcp_close_socket((rp_tcp_ext_t *) &resource->handler);
}

PHP_METHOD(respond_server_tcp, on)
{
    zval *self = getThis();
    rp_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_tcp_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }
    fprintf(stderr, "on: %.*s\n", event_len, event);
    rp_event_emitter_on(&resource->event_hook, event, event_len, hook);
//    zend_print_zval_r(&resource->event_hook.hook, 0);
}

PHP_METHOD(respond_server_tcp, off)
{
    zval *self = getThis();
    rp_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_tcp_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }
    fprintf(stderr, "off: %.*s\n", event_len, event);
    rp_event_emitter_off(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_server_tcp, removeListeners)
{
    zval *self = getThis();
    rp_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_tcp_ext_t);
    const char *event;
    size_t event_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &event, &event_len)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event, event_len);
}

PHP_METHOD(respond_server_tcp, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_tcp_ext_t);
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

PHP_METHOD(respond_server_tcp, emit)
{
    zval *self = getThis();
    rp_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_tcp_ext_t);
    const char *event;
    size_t event_len;
    zval *params;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &params)) {
        return;
    }
    fprintf(stderr, "emit %.*s\n", event_len, event);
    rp_event_emitter_emit(&resource->event_hook, event, event_len, params);
    RETURN_ZVAL(params, 1, 0);
}
