#include "respondphp.h"
#include "server/udp.h"
DECLARE_FUNCTION_ENTRY(respond_server_udp) =
{
    PHP_ME(respond_server_udp, __construct, ARGINFO(respond_server_udp, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_udp, send, ARGINFO(respond_server_udp, send), ZEND_ACC_PUBLIC)
    PHP_ME(respond_server_udp, close, NULL, ZEND_ACC_PUBLIC)
    TRAIT_FUNCTION_ENTRY_ME(respond_server_udp, event_emitter)
    PHP_FE_END
};

static zend_object *create_respond_server_udp_resource(zend_class_entry *class_type);
static void free_respond_server_udp_resource(zend_object *object);
static void send_cb(rp_reactor_t *reactor, const char *data, size_t data_len, const struct sockaddr *addr);
static void recv_cb(rp_reactor_t *reactor, ssize_t nread, const uv_buf_t* buf, const struct sockaddr *addr, unsigned flags);
static void data_recv_cb(zend_object *server, const char *data, size_t data_len, const struct sockaddr *addr, unsigned flags);
static void releaseResource(rp_udp_ext_t *resource);
static void server_init(rp_reactor_t *reactor);

static void server_init(rp_reactor_t *reactor)
{
    uv_udp_init(&main_loop, &reactor->handler.udp);
    uv_udp_bind(&reactor->handler.udp, (const struct sockaddr*) &reactor->addr, 0);
    uv_udp_recv_start(&reactor->handler.udp, rp_alloc_buffer, reactor->cb.dgram.recv);
}

CLASS_ENTRY_FUNCTION_D(respond_server_udp)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_udp, "Respond\\Server\\Udp", create_respond_server_udp_resource);
    OBJECT_HANDLER(respond_server_udp).offset = XtOffsetOf(rp_udp_ext_t, zo);
    OBJECT_HANDLER(respond_server_udp).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_udp).free_obj = free_respond_server_udp_resource;
    zend_class_implements(CLASS_ENTRY(respond_server_udp), 1, CLASS_ENTRY(respond_event_event_emitter_interface));
}

static void releaseResource(rp_udp_ext_t *resource)
{
}

static void send_cb(rp_reactor_t *reactor, const char *data, size_t data_len, const struct sockaddr *addr)
{
    char addr_str[INET6_ADDRSTRLEN];
    uint16_t port;
    rp_write_req_t *req = rp_make_write_req(data, data_len);
    sock_addr(addr, addr_str, INET6_ADDRSTRLEN, &port);
    int ret = uv_udp_send(req, &reactor->handler, &req->buf, 1, addr, rp_close_cb_release);
    fprintf(stderr, "req send start (%d) %s %p %.*s %s\n", ret, uv_strerror(ret), req, req->buf.len, req->buf.base, addr_str);
}

static void recv_cb(rp_reactor_t *reactor, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    size_t sockaddr_size;

    rp_reactor_data_send_req_t *req;
    size_t size_of_req_data;
    if (nread > 0) {
        if(addr->sa_family == AF_INET) {
            sockaddr_size = sizeof(struct sockaddr_in);
        }
        else {
            sockaddr_size = sizeof(struct sockaddr_in6);
        }
        size_of_req_data = sizeof(rp_reactor_data_send_req_t) + nread;
        req = rp_malloc(size_of_req_data);
        req->type = RP_RECV;
        req->payload.recv.flags = flags;
        memcpy(&req->payload.recv.addr, addr, sockaddr_size);
        req->payload.recv.data_len = nread;
        memcpy(&req->payload.recv.data, buf->base, nread);
        int ret = rp_reactor_data_send(reactor, rp_close_cb_release, req, size_of_req_data);
        fprintf(stderr, "recv data send %p %d %s\n", reactor, ret, uv_strerror(ret));
        rp_free(req);
    }
    rp_free(buf->base);
}

static void data_recv_cb(zend_object *server, const char *data, size_t data_len, const struct sockaddr *addr, unsigned flags)
{
    rp_udp_ext_t *resource = FETCH_RESOURCE(server, rp_udp_ext_t);
    char addr_str[40];
    uint16_t port;
    zval param[4];
fprintf(stderr, "recv: %d %d %d\n", addr->sa_family, AF_INET, AF_INET6);
    sock_addr(addr, addr_str, sizeof(addr_str), &port);
    ZVAL_OBJ(&param[0], server);
    ZVAL_STRING(&param[1], addr_str);
    ZVAL_LONG(&param[2], port);
    ZVAL_STRINGL(&param[3], data, data_len);
    rp_event_emitter_emit(&resource->event_hook, ZEND_STRL("recv"), 4, param);
    ZVAL_PTR_DTOR(&param[1]);
    ZVAL_PTR_DTOR(&param[3]);
}

static zend_object *create_respond_server_udp_resource(zend_class_entry *ce)
{
    rp_udp_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_udp_ext_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_udp);
    rp_event_hook_init(&resource->event_hook);
    return &resource->zo;
}

static void free_respond_server_udp_resource(zend_object *object)
{
    rp_udp_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_udp_ext_t);
    releaseResource(resource);
    rp_event_hook_destroy(&resource->event_hook);
    zend_object_std_dtor(object);
}

PHP_METHOD(respond_server_udp, __construct)
{
    long port;
    zval *self = getThis();
    zend_string *host;
    rp_reactor_addr_t addr;
    rp_reactor_t *reactor;
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &host, &port)) {
        return;
    }

    if(memchr(host->val, ':', host->len) == NULL) {
        if (uv_ip4_addr(host->val, port & 0xffff, &addr.sockaddr) != 0) {
            return;
        }
    }
    else {
        if (uv_ip6_addr(host->val, port & 0xffff, &addr.sockaddr6) != 0) {
            return;
        }
    }

    reactor = rp_reactors_add(self);
    memcpy(&reactor->addr, &addr, sizeof(rp_reactor_addr_t));
    reactor->type = RP_UDP;
    reactor->server_init_cb = server_init;
    reactor->cb.dgram.recv = (rp_recv_cb) recv_cb;
    reactor->cb.dgram.send = (rp_send_cb) send_cb;
    reactor->cb.dgram.data_recv = (rp_data_recv_cb) data_recv_cb;
    resource->reactor = reactor;
}

PHP_METHOD(respond_server_udp, send)
{
    zval *self = getThis();
    zend_string *host, *data;
    long port;
    rp_reactor_addr_t addr;
    rp_reactor_data_send_req_t *req;
    size_t size_of_req_data;

    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "SlS", &host, &port, &data)) {
        return;
    }

    if(memchr(host->val, ':', host->len) == NULL) {
        if (uv_ip4_addr(host->val, port & 0xffff, &addr.sockaddr) != 0) {
            return;
        }
    }
    else {
        if (uv_ip6_addr(host->val, port & 0xffff, &addr.sockaddr6) != 0) {
            return;
        }
    }

    size_of_req_data = sizeof(rp_reactor_data_send_req_t) + data->len;
    req = rp_malloc(size_of_req_data);
    req->type = RP_SEND;
    memcpy(&req->payload.send.addr, &addr, sizeof(rp_reactor_addr_t));
    req->payload.send.data_len = data->len;
    memcpy(&req->payload.send.data, data->val, data->len);
    rp_reactor_data_send(resource->reactor, rp_close_cb_release, req, size_of_req_data);
//        fprintf(stderr, "recv data send %p %d %s\n", reactor, ret, uv_strerror(ret));
    rp_free(req);
}

PHP_METHOD(respond_server_udp, close)
{
//    zval *self = getThis();
//    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
}

PHP_METHOD(respond_server_udp, on)
{
    zval *self = getThis();
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }
    rp_event_emitter_on(&resource->event_hook, event->val, event->len, hook);
//    zend_print_zval_r(&resource->event_hook.hook, 0);
}

PHP_METHOD(respond_server_udp, off)
{
    zval *self = getThis();
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }
    rp_event_emitter_off(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_server_udp, removeListeners)
{
    zval *self = getThis();
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
    zend_string *event;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &event)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event->val, event->len);
}

PHP_METHOD(respond_server_udp, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
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

PHP_METHOD(respond_server_udp, emit)
{
    zval *self = getThis();
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
        Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}
