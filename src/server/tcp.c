#include "respondphp.h"
#include "server/tcp.h"

static void client_accept_close_cb(uv_handle_t* handle)
{
    free(handle);
}

void tcp_close_socket(uv_tcp_ext_t *handle)
{
    if(handle->flag & UV_TCP_CLOSING_START) {
         return;
    }
    handle->flag |= UV_TCP_CLOSING_START;
    uv_close((uv_handle_t *) handle, tcp_close_cb);
}

void setSelfReference(uv_tcp_ext_t *resource)
{
    if(resource->flag & UV_TCP_HANDLE_INTERNAL_REF) {
        return;
    }
    Z_ADDREF(resource->object);
    resource->flag |= UV_TCP_HANDLE_INTERNAL_REF;
}

CLASS_ENTRY_FUNCTION_D(respond_server_tcp)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_tcp, "Respond\\Server\\Tcp", create_respond_server_tcp_resource);
    OBJECT_HANDLER(respond_server_tcp).offset = offsetof(uv_tcp_ext_t, zo);
    OBJECT_HANDLER(respond_server_tcp).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_tcp).free_obj = free_respond_server_tcpResource;
}

void releaseResource(uv_tcp_ext_t *resource)
{
    if(resource->flag & UV_TCP_HANDLE_INTERNAL_REF) {
        resource->flag &= ~UV_TCP_HANDLE_INTERNAL_REF;
        zval_ptr_dtor(&resource->object);
    }    
}

static void tcp_close_cb(uv_handle_t* handle)
{
    releaseResource((uv_tcp_ext_t *) handle);
}

static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if(nread > 0){
        fprintf(stderr, "recv(%d)\n %.*s\n", nread, nread, buf->base);
    }
    
    free(buf->base);
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
    uv_tcp_ext_t *resource;
    resource = ALLOC_RESOURCE(uv_tcp_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_tcp);
    return &resource->zo;
}

static void free_respond_server_tcpResource(zend_object *object)
{
    uv_tcp_ext_t *resource;
    resource = FETCH_RESOURCE(object, uv_tcp_ext_t);
    releaseResource(resource);
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
    
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);    

    if(((uv_tcp_ext_t *) &resource->handler)->flag & UV_TCP_HANDLE_START){
        RETURN_LONG(-1);
    }
    
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
    RETURN_LONG(ret);
}


static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static void accepted_cb(rp_client_t *client)
{
    uv_read_start((uv_stream_t*) client, alloc_buffer, read_cb);    
}

PHP_METHOD(respond_server_tcp, close)
{
    long ret;
    zval *self = getThis();
    uv_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, uv_tcp_ext_t);
    tcp_close_socket((uv_tcp_ext_t *) &resource->handler);
}
