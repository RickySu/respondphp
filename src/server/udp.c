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
static void send_cb(rp_reactor_t *reactor, rp_reactor_data_send_req_payload_send_t *payload, rp_stream_t *result_stream);
static void worker_recv_cb(rp_reactor_t *reactor, ssize_t nread, const uv_buf_t* buf, const struct sockaddr *addr, unsigned flags);
static void actor_recv_cb(rp_reactor_t *reactor, ssize_t nread, const uv_buf_t* buf, const struct sockaddr *addr, unsigned flags);
static void data_actor_recv_cb(zend_object *server, rp_reactor_data_send_req_payload_recv_t *recv_req);
static void releaseResource(rp_udp_ext_t *resource);
static void server_init(rp_reactor_t *reactor);
static rp_udp_send_result_t *udp_send_data(rp_reactor_t *reactor, zend_string *data, rp_reactor_addr_t *addr);
static void udp_send_cb(rp_udp_send_t *udp_send, int status);
static rp_udp_send_result_t *ipc_udp_send_data(rp_reactor_t *reactor, zend_string *data, rp_reactor_addr_t *addr);
static void ipc_udp_send_cb(rp_udp_send_t *udp_send, int status);
static void ipc_udp_send_result_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf);
static void actor_udp_init(rp_udp_ext_t *resource, zval *self, zend_string *host, long port);
static void worker_udp_init(rp_udp_ext_t *resource, zval *self, zend_string *host, long port);
static void result_send_cb(uv_write_t* req, int status);
static void ipc_udp_send_result_free_cb(uv_handle_t *handle);

static void server_init(rp_reactor_t *reactor)
{
    uv_os_fd_t sockfd;
    uv_udp_init(&main_loop, &reactor->handler.udp);
    sockfd = socket(reactor->addr.sockaddr.sin_family, SOCK_DGRAM, 0);

#ifdef HAVE_REUSEPORT
    int status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int));
#endif

    uv_udp_open(&reactor->handler.udp, sockfd);

    if(reactor->addr.sockaddr.sin_family != AF_UNSPEC) {
        uv_udp_bind(&reactor->handler.udp, (const struct sockaddr *) &reactor->addr, 0);
    }

    uv_udp_recv_start(&reactor->handler.udp, rp_alloc_buffer, reactor->cb.dgram.recv);

    uint16_t port;
    char addr_str[INET6_ADDRSTRLEN];
    sock_addr(&reactor->addr, addr_str, sizeof(addr_str), &port);
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

static void result_send_cb(uv_write_t* req, int status)
{
    uv_close(req->handle, rp_free_cb);
    rp_free(req);
}

static void send_cb(rp_reactor_t *reactor, rp_reactor_data_send_req_payload_send_t *payload, rp_stream_t *result_stream)
{
    int ret;
    rp_udp_send_t *udp_send;
    udp_send = rp_malloc(sizeof(rp_udp_send_t));
    udp_send->buf.base = payload->data;
    udp_send->buf.len = payload->data_len;
    udp_send->payload = payload;
    udp_send->result_stream = result_stream;


#ifdef HAVE_REUSEPORT
    ret = uv_udp_send(udp_send, &reactor->handler, &udp_send->buf, 1, &payload->addr, udp_send_cb);
#else
    ret = uv_udp_send(udp_send, &reactor->handler, &udp_send->buf, 1, &payload->addr, ipc_udp_send_cb);
#endif

    char addr_str[INET6_ADDRSTRLEN];
    uint16_t port;
    sock_addr(&payload->addr, addr_str, INET6_ADDRSTRLEN, &port);
}

static void actor_recv_cb(rp_reactor_t *reactor, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
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
        int ret = rp_reactor_data_send(reactor, rp_free_cb, req, size_of_req_data);
        rp_free(req);
    }
    rp_free(buf->base);
}

static void worker_recv_cb(rp_reactor_t *reactor, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    size_t sockaddr_size;
    rp_reactor_data_send_req_payload_recv_t *recv;

    if (nread > 0) {
        if(addr->sa_family == AF_INET) {
            sockaddr_size = sizeof(struct sockaddr_in);
        }
        else {
            sockaddr_size = sizeof(struct sockaddr_in6);
        }

        recv = rp_malloc(sizeof(rp_reactor_data_send_req_payload_recv_t));
        recv->flags = flags;
        memcpy(&recv->addr, addr, sockaddr_size);
        recv->data_len = nread;
        memcpy(&recv->data, buf->base, nread);
        reactor->cb.dgram.data_recv(reactor->server, recv);
        rp_free(recv);
    }
    rp_free(buf->base);
}

static void data_actor_recv_cb(zend_object *server, rp_reactor_data_send_req_payload_recv_t *recv_req)
{
    rp_udp_ext_t *resource = FETCH_RESOURCE(server, rp_udp_ext_t);
    char addr_str[40];
    uint16_t port;
    zval param[4];
    sock_addr(&recv_req->addr, addr_str, sizeof(addr_str), &port);
    ZVAL_OBJ(&param[0], server);
    ZVAL_STRING(&param[1], addr_str);
    ZVAL_LONG(&param[2], port);
    ZVAL_STRINGL(&param[3],recv_req->data, recv_req->data_len);
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

static void actor_udp_init(rp_udp_ext_t *resource, zval *self, zend_string *host, long port)
{
    rp_reactor_addr_t addr;
    rp_reactor_t *reactor;
    if(port != -1) {

        if(!rp_addr(&addr, host, port & 0xffff)){
            return;
        }

        reactor = rp_reactors_add_new(self);
        memcpy(&reactor->addr, &addr, sizeof(rp_reactor_addr_t));
    }
    else{
        reactor = rp_reactors_add_new(self);
    }
    reactor->type = RP_UDP;

#ifdef HAVE_REUSEPORT
    reactor->worker_init_cb = server_init;
    reactor->cb.dgram.recv = (rp_recv_cb) worker_recv_cb;
#else
    reactor->server_init_cb = server_init;
    reactor->cb.dgram.recv = (rp_recv_cb) actor_recv_cb;
#endif

    reactor->cb.dgram.send = (rp_send_cb) send_cb;
    reactor->cb.dgram.data_recv = (rp_data_recv_cb) data_actor_recv_cb;
    resource->reactor = reactor;
}

static void worker_udp_init(rp_udp_ext_t *resource, zval *self, zend_string *host, long port)
{
    rp_reactor_addr_t addr;
    rp_reactor_t *reactor = rp_malloc(sizeof(rp_reactor_t));
    rp_reactor_init(reactor);
    if(port != -1) {

        if(!rp_addr(&addr, host, port & 0xffff)){
            rp_free(reactor);
            return;
        }

        memcpy(&reactor->addr, &addr, sizeof(rp_reactor_addr_t));
    }

    Z_ADDREF_P(self);
    reactor->server = Z_OBJ_P(self);

    reactor->type = RP_UDP;
    reactor->worker_init_cb = server_init;
    reactor->cb.dgram.recv = (rp_recv_cb) worker_recv_cb;
    reactor->cb.dgram.send = (rp_send_cb) send_cb;
    reactor->cb.dgram.data_recv = (rp_data_recv_cb) data_actor_recv_cb;
    resource->reactor = reactor;
    resource->flag |= RP_UDP_SERVER_WORKER_MODE;
    server_init(reactor);
}

PHP_METHOD(respond_server_udp, __construct)
{
    long port = -1;
    zend_string *host = NULL;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|lS", &port, &host)) {
        return;
    }

    if(rp_get_task_type() == ACTOR){
        actor_udp_init(FETCH_OBJECT_RESOURCE(getThis(), rp_udp_ext_t), getThis(), host, port);
        return;
    }

    worker_udp_init(FETCH_OBJECT_RESOURCE(getThis(), rp_udp_ext_t), getThis(), host, port);
}

PHP_METHOD(respond_server_udp, send)
{
    zval *self = getThis();
    zend_string *host, *data;
    long port;
    rp_reactor_addr_t addr;
    rp_udp_send_result_t *result;

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

#if HAVE_REUSEPORT
    result = udp_send_data(resource->reactor, data, &addr);
#else
    if(resource->flag & RP_UDP_SERVER_WORKER_MODE){
        result = udp_send_data(resource->reactor, data, &addr);
    }
    else {
        result = ipc_udp_send_data(resource->reactor, data, &addr);
    }
#endif

    RETVAL_ZVAL(&result->promise, 1, 0);
}

static void ipc_udp_send_result_free_cb(uv_handle_t *handle)
{
    rp_udp_send_result_t *result = FETCH_POINTER(handle, rp_udp_send_result_t, worker_pipe);
    rp_free(result);
}

static void ipc_udp_send_result_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf)
{
    rp_udp_send_result_t *result = FETCH_POINTER(pipe, rp_udp_send_result_t, worker_pipe);
    zval send_result;
    if(status >= 0){
        ZVAL_LONG(&send_result, *((int *) buf->base));
        rp_resolve_promise(&result->promise, &send_result);
    }
    else{
        rp_reject_promise_long(&result->promise, status);
    }
    ZVAL_PTR_DTOR(&result->promise);
    rp_free(buf->base);
    uv_close((uv_handle_t *) &result->worker_pipe, (uv_close_cb) ipc_udp_send_result_free_cb);
}

static void udp_send_cb(rp_udp_send_t *udp_send, int status)
{
    rp_udp_send_result_t *result = (rp_udp_send_result_t *) udp_send;
    zval send_result;

    if(status >= 0){
        ZVAL_LONG(&send_result, status);
        rp_resolve_promise(&result->promise, &send_result);
    }
    else{
        rp_reject_promise_long(&result->promise, status);
    }

    ZVAL_PTR_DTOR(&result->promise);
    zend_string_release(result->data);
    rp_free(result);
}

static rp_udp_send_result_t *udp_send_data(rp_reactor_t *reactor, zend_string *data, rp_reactor_addr_t *addr)
{
    rp_udp_send_result_t *result;
    uv_buf_t buf;
    result = emalloc(sizeof(rp_udp_send_result_t));
    rp_make_promise_object(&result->promise);
    result->data = data;
    buf.base = data->val;
    buf.len = data->len;
    zend_string_addref(data);
    int ret = uv_udp_send(&result->send, &reactor->handler, &buf, 1, addr, udp_send_cb);
    return result;
}

static void ipc_udp_send_cb(rp_udp_send_t *udp_send, int status)
{
    rp_write_req_t *req;
    rp_reactor_data_send_req_t *send_req = FETCH_POINTER(udp_send->payload, rp_reactor_data_send_req_t, payload);
    rp_reactor_ext_t *reactor_ext = FETCH_POINTER(send_req, rp_reactor_ext_t, data);
    req = rp_make_write_req((char *) &status, sizeof(status));
    uv_write(req, udp_send->result_stream, &req->buf, 1, (uv_write_cb) result_send_cb);
    rp_free(reactor_ext);
    rp_free(udp_send);
}

static rp_udp_send_result_t *ipc_udp_send_data(rp_reactor_t *reactor, zend_string *data, rp_reactor_addr_t *addr)
{
    int fd[2];
    rp_udp_send_result_t *result;
    rp_reactor_data_send_req_t *req;
    size_t size_of_req_data;
    size_of_req_data = sizeof(rp_reactor_data_send_req_t) + data->len;
    req = rp_malloc(size_of_req_data);
    req->payload.send.sender = getpid();
    req->type = RP_SEND;
    memcpy(&req->payload.send.addr, addr, sizeof(rp_reactor_addr_t));
    req->payload.send.data_len = data->len;
    memcpy(&req->payload.send.data, data->val, data->len);

    result = emalloc(sizeof(rp_udp_send_result_t));
    rp_make_promise_object(&result->promise);

    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);

    uv_pipe_init(&main_loop, &result->actor_pipe, 0);
    uv_pipe_open(&result->actor_pipe, fd[0]);

    uv_pipe_init(&main_loop, &result->worker_pipe, 0);
    uv_pipe_open(&result->worker_pipe, fd[1]);
    uv_read_start(&result->worker_pipe, rp_alloc_buffer, (uv_read_cb) ipc_udp_send_result_receive);
    rp_reactor_ipc_send_ex(reactor, (uv_stream_t *) &result->actor_pipe, NULL, req, size_of_req_data, &ipc_pipe);
    rp_free(req);
    return result;
}

static void udp_close_cb(rp_reactor_t *reactor)
{
    zend_object_ptr_dtor(reactor->server);
    rp_free(reactor);
}

PHP_METHOD(respond_server_udp, close)
{
    zval *self = getThis();
    rp_udp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_udp_ext_t);
    if(resource->flag & RP_UDP_SERVER_WORKER_MODE){
        uv_close(&resource->reactor->handler, (uv_close_cb) udp_close_cb);
    }
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
