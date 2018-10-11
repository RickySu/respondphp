#include "respondphp.h"
#include "server/udp.h"

static zend_object *create_respond_server_udp_resource(zend_class_entry *class_type);
static void free_respond_server_udp_resource(zend_object *object);
static void client_accept_close_cb(uv_handle_t* handle);
static void recv_cb(rp_reactor_t *reactor, ssize_t nread, const uv_buf_t* buf, const struct sockaddr *addr, unsigned flags);
static void data_recv_cb(zend_object *server, const char *data, size_t data_len, const struct sockaddr *addr, unsigned flags);
static void releaseResource(rp_udp_ext_t *resource);

static void client_accept_close_cb(uv_handle_t* handle)
{
    rp_free(handle);
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

static void recv_cb(rp_reactor_t *reactor, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    rp_reactor_data_send_req_t *req;
    size_t size_of_req_data;
    if (nread > 0) {
        size_of_req_data = sizeof(rp_reactor_data_send_req_t) + nread;
        req = rp_malloc(size_of_req_data);
        req->type = RP_DATA;
        req->payload.recv.flags = flags;
        memcpy(&req->payload.recv.addr, addr, sizeof(const struct sockaddr));
        req->payload.recv.data_len = nread;
        memcpy(&req->payload.recv.data, buf->base, nread);
        int ret = rp_reactor_data_send(reactor, client_accept_close_cb, req, size_of_req_data);
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

    sock_addr(addr, addr_str, sizeof(addr_str), &port);
    snprintf(&addr_str[strlen(addr_str)], 6, ":%d", (int)port);
    fprintf(stderr, "recv data recv %d from %s:%.*s\n", getpid(), addr_str, data_len, data);
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
    long ret, port;
    zval *self = getThis();
    const char *host = NULL;
    size_t host_len;
    char cstr_host[40];

    rp_reactor_t *reactor;
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sl", &host, &host_len, &port)) {
        return;
    }
    
    if(host_len == 0 || host_len >= 30) {
        return;
    }

    memcpy(cstr_host, host, host_len);
    cstr_host[host_len] = '\0';

    reactor = rp_reactor_add();

    if(strchr(cstr_host, ':') == NULL) {
        if ((ret = uv_ip4_addr(cstr_host, port & 0xffff, &reactor->addr.sockaddr)) != 0) {
            return;
        }
    }
    else {
        if ((ret = uv_ip6_addr(cstr_host, port & 0xffff, &reactor->addr.sockaddr6)) != 0) {
            return;
        }
    }

    reactor->type = RP_UDP;
    reactor->cb.dgram.recv = (rp_recv_cb) recv_cb;
    reactor->cb.dgram.data_recv = (rp_data_recv_cb) data_recv_cb;
    reactor->server = &resource->zo;
    resource->reactor = reactor;
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
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }
    rp_event_emitter_on(&resource->event_hook, event, event_len, hook);
//    zend_print_zval_r(&resource->event_hook.hook, 0);
}

PHP_METHOD(respond_server_udp, off)
{
    zval *self = getThis();
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
    const char *event;
    size_t event_len;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &hook)) {
        return;
    }
    rp_event_emitter_off(&resource->event_hook, event, event_len, hook);
}

PHP_METHOD(respond_server_udp, removeListeners)
{
    zval *self = getThis();
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
    const char *event;
    size_t event_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &event, &event_len)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event, event_len);
}

PHP_METHOD(respond_server_udp, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
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
