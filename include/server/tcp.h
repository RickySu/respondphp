#ifndef _RP_SERVER_TCP_H
#define _RP_SERVER_TCP_H

#define UV_TCP_HANDLE_INTERNAL_REF 1
#define UV_TCP_HANDLE_START (1<<1)
#define UV_TCP_READ_START (1<<2)
#define UV_TCP_CLOSING_START (1<<3)
#define UV_TCP_WRITE_CALLBACK_ENABLE (1<<4)

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_tcp, __construct), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

typedef struct uv_tcp_ext_s{
    uv_tcp_t handler;
    uint flag;
    uv_connect_t connect_req;
    uv_shutdown_t shutdown_req;
    char *sockAddr;
    int sockPort;
    char *peerAddr;
    int peerPort;
    zval object;
    zend_object zo;
} uv_tcp_ext_t;

typedef struct write_req_s{
    uv_write_t uv_write;
    uv_buf_t buf;
} write_req_t;

static zend_object *create_respond_server_tcp_resource(zend_class_entry *class_type);
static void free_respond_server_tcpResource(zend_object *object);
static void client_accept_close_cb(uv_handle_t* handle);
static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
static void connection_cb(rp_reactor_t *reactor, int status);
static void accepted_cb(rp_client_t *client);
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void releaseResource(uv_tcp_ext_t *resource);
static void tcp_close_cb(uv_handle_t* handle);
void tcp_close_socket(uv_tcp_ext_t *handle);
void setSelfReference(uv_tcp_ext_t *resource);

PHP_METHOD(respond_server_tcp, close);
PHP_METHOD(respond_server_tcp, __construct);

DECLARE_FUNCTION_ENTRY(respond_server_tcp) = {
    PHP_ME(respond_server_tcp, __construct, ARGINFO(respond_server_tcp, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_tcp, close, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
#endif
