#include "respondphp.h"
#include "internal/socket_connector.h"
#include "connector/tcp.h"

static void connect_async_cb(rp_connector_t *connector);

DECLARE_FUNCTION_ENTRY(respond_connector_tcp) =
{
    PHP_ME(respond_connector_tcp, __construct, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
    PHP_ME(respond_connector_tcp, connect, ARGINFO(respond_connector_tcp, connect), ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_connector_tcp)
{
    REGISTER_CLASS(respond_connector_tcp, "Respond\\Connector\\Tcp");
    zend_class_implements(CLASS_ENTRY(respond_connector_tcp), 1, CLASS_ENTRY(respond_socket_connector_interface));
}

static void connect_async_cb(rp_connector_t *connector)
{
    uv_tcp_t *uv_tcp;
    uv_tcp = rp_malloc(sizeof(uv_tcp_t));
    uv_tcp_init(&main_loop, uv_tcp);
    rp_socket_connect(connector, uv_tcp, &connector->addr);
    fprintf(stderr, "connect: %p %p\n", connector, uv_tcp);
}

PHP_METHOD(respond_connector_tcp, __construct)
{

}

PHP_METHOD(respond_connector_tcp, connect)
{
    zend_string *host;
    zend_long port;
    rp_connector_t *connector;
    int err;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &host, &port)) {
        return;
    }

    connector = rp_malloc(sizeof(rp_connector_t));
    rp_make_promise_object(&connector->promise);
    RETVAL_ZVAL(&connector->promise, 1, 0);

    if(memchr(host->val, ':', host->len) == NULL) {
        err = uv_ip4_addr(host->val, port & 0xffff, &connector->addr.sockaddr);
    }
    else {
        err = uv_ip6_addr(host->val, port & 0xffff, &connector->addr.sockaddr6);
    }

    if(err != 0){
        rp_reject_promise_long(&connector->promise, err);
        rp_free(connector);
        return;
    }

    rp_reactor_async_init((rp_reactor_async_init_cb) connect_async_cb, connector);
}
