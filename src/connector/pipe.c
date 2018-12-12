#include "respondphp.h"
#include "internal/socket_connector.h"
#include "connector/pipe.h"

DECLARE_FUNCTION_ENTRY(respond_connector_pipe) =
{
    PHP_ME(respond_connector_pipe, __construct, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
    PHP_ME(respond_connector_pipe, connect, ARGINFO(respond_connector_pipe, connect), ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

static void connect_async_cb(rp_connector_t *connector);

CLASS_ENTRY_FUNCTION_D(respond_connector_pipe)
{
    REGISTER_CLASS(respond_connector_pipe, "Respond\\Connector\\Pipe");
    zend_class_implements(CLASS_ENTRY(respond_connector_pipe), 1, CLASS_ENTRY(respond_socket_connector_interface));
}

static void connect_async_cb(rp_connector_t *connector)
{
    uv_pipe_t *uv_pipe;
    uv_pipe = rp_malloc(sizeof(uv_pipe_t));
    uv_pipe_init(&main_loop, uv_pipe, 0);
    rp_socket_connect(connector, uv_pipe, &connector->addr);
    fprintf(stderr, "connect: %p %p\n", connector, uv_pipe);
}

PHP_METHOD(respond_connector_pipe, __construct)
{

}

PHP_METHOD(respond_connector_pipe, connect)
{
    rp_connector_t *connector;

    connector = rp_malloc(sizeof(rp_connector_t));
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &connector->addr.socket_path)) {
        rp_free(connector);
        return;
    }

    rp_make_promise_object(&connector->promise);
    RETVAL_ZVAL(&connector->promise, 1, 0);
    zend_string_addref(connector->addr.socket_path);
    rp_reactor_async_init((rp_reactor_async_cb) connect_async_cb, connector);
}
