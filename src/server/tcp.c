#include "respondphp.h"
#include "server/tcp.h"
DECLARE_FUNCTION_ENTRY(respond_server_tcp) =
{
    PHP_ME(respond_server_tcp, __construct, ARGINFO(respond_server_tcp, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_tcp, close, NULL, ZEND_ACC_PUBLIC)
    TRAIT_FUNCTION_ENTRY_ME(respond_server_tcp, event_emitter)
    PHP_FE_END
};

static zend_object *create_respond_server_tcp_resource(zend_class_entry *class_type);
static void free_respond_server_tcp_resource(zend_object *object);
static void connection_cb(rp_reactor_t *reactor, int status);
static void accepted_cb(zend_object *server, rp_stream_t *client);
static void releaseResource(rp_server_tcp_ext_t *resource);
static void server_init(rp_reactor_t *reactor);

CLASS_ENTRY_FUNCTION_D(respond_server_tcp)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_tcp, "Respond\\Server\\Tcp", create_respond_server_tcp_resource);
    OBJECT_HANDLER(respond_server_tcp).offset = XtOffsetOf(rp_server_tcp_ext_t, zo);
    OBJECT_HANDLER(respond_server_tcp).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_tcp).free_obj = free_respond_server_tcp_resource;
    zend_class_implements(CLASS_ENTRY(respond_server_tcp), 1, CLASS_ENTRY(respond_stream_server_interface));
}

static void server_init(rp_reactor_t *reactor)
{
    char addr_str[INET6_ADDRSTRLEN];
    uint16_t port;
    uv_tcp_init(&main_loop, &reactor->handler.tcp);
    uv_tcp_bind(&reactor->handler.tcp, (const struct sockaddr *) &reactor->addr, 0);
    uv_listen((uv_stream_t *) &reactor->handler.tcp, SOMAXCONN, reactor->cb.stream.connection);
    sock_addr(&reactor->addr, addr_str, sizeof(addr_str), &port);
    fprintf(stderr, "tcp listen: %s:%d\n", addr_str, port);
}

static void releaseResource(rp_server_tcp_ext_t *resource)
{
}

static void connection_cb(rp_reactor_t *reactor, int status)
{
    if (status < 0) {
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) rp_malloc(sizeof(uv_tcp_t));
    uv_tcp_init(&main_loop, client);
    
    if (uv_accept((uv_stream_t *) &reactor->handler.tcp, (uv_stream_t*) client) == 0) {
        rp_reactor_ipc_send(reactor, (uv_stream_t *) client, rp_close_cb_release);
        return;
    }
    
    uv_close((uv_handle_t *) client, rp_close_cb_release);
}

static zend_object *create_respond_server_tcp_resource(zend_class_entry *ce)
{
    rp_server_tcp_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_server_tcp_ext_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_tcp);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_server_tcp_resource(zend_object *object)
{
    rp_server_tcp_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_server_tcp_ext_t);
    releaseResource(resource);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_std_dtor(object);
}

PHP_METHOD(respond_server_tcp, __construct)
{
    long ret, port;
    zval *self = getThis();
    zend_string *host;
    rp_reactor_addr_t addr;
    rp_reactor_t *reactor;
    rp_server_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_tcp_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &host, &port)) {
        return;
    }

    if(memchr(host->val, ':', host->len) == NULL) {
        if ((ret = uv_ip4_addr(host->val, port & 0xffff, &addr.sockaddr)) != 0) {
            return;
        }
    }
    else {
        if ((ret = uv_ip6_addr(host->val, port & 0xffff, &addr.sockaddr6)) != 0) {
            return;
        }
    }

    fprintf(stderr, "tcp:%p\n", &resource->event_hook);

    reactor = rp_reactors_add(self);
    memcpy(&reactor->addr, &addr, sizeof(rp_reactor_addr_t));
    reactor->type = RP_TCP;
    reactor->server_init_cb = server_init;
    reactor->cb.stream.connection = (rp_connection_cb) connection_cb;
    reactor->cb.stream.accepted = (rp_accepted_cb) accepted_cb;
    resource->reactor = reactor;
}

static void accepted_cb(zend_object *server, rp_stream_t *client)
{
    zval connection;
    rp_server_tcp_ext_t *resource = FETCH_RESOURCE(server, rp_server_tcp_ext_t);
    rp_connection_connection_factory(client, &connection);
    RP_ASSERT(zval_refcount_p(&connection) == 1);
    rp_event_emitter_emit_internal(&resource->event_hook, ZEND_STRL("connect"), 1, &connection);
    RP_ASSERT(zval_refcount_p(&connection) >= 1);
}

PHP_METHOD(respond_server_tcp, close)
{
//    zval *self = getThis();
//    rp_server_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_tcp_ext_t);
}

PHP_METHOD(respond_server_tcp, on)
{
    zval *self = getThis();
    rp_server_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_tcp_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }
    rp_event_emitter_on(&resource->event_hook, event->val, event->len, hook);
//    zend_print_zval_r(&resource->event_hook.hook, 0);
}

PHP_METHOD(respond_server_tcp, off)
{
    zval *self = getThis();
    rp_server_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_tcp_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }
    rp_event_emitter_off(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_server_tcp, removeListeners)
{
    zval *self = getThis();
    rp_server_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_tcp_ext_t);
    zend_string *event;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &event)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event->val, event->len);
}

PHP_METHOD(respond_server_tcp, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_server_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_tcp_ext_t);
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

PHP_METHOD(respond_server_tcp, emit)
{
    zval *self = getThis();
    rp_server_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_tcp_ext_t);
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
        Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}
