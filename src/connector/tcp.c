#include "respondphp.h"
#include "internal/socket_connector.h"
#include "connector/tcp.h"

DECLARE_FUNCTION_ENTRY(respond_connector_tcp) =
{
    PHP_ME(respond_connector_tcp, connect, ARGINFO(respond_connector_tcp, connect), ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static zend_object *create_respond_connector_tcp_resource(zend_class_entry *class_type);
static void free_respond_connector_tcp_resource(zend_object *object);
static void releaseResource(rp_connector_tcp_ext_t *resource);
static void connect_async_cb(rp_connector_t *connector);

CLASS_ENTRY_FUNCTION_D(respond_connector_tcp)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_connector_tcp, "Respond\\Connector\\Tcp", create_respond_connector_tcp_resource);
    OBJECT_HANDLER(respond_connector_tcp).offset = XtOffsetOf(rp_connector_tcp_ext_t, zo);
    OBJECT_HANDLER(respond_connector_tcp).clone_obj = NULL;
    OBJECT_HANDLER(respond_connector_tcp).free_obj = free_respond_connector_tcp_resource;
    zend_class_implements(CLASS_ENTRY(respond_connector_tcp), 1, CLASS_ENTRY(respond_socket_connector_interface));
}

static void releaseResource(rp_connector_tcp_ext_t *resource)
{
}

static zend_object *create_respond_connector_tcp_resource(zend_class_entry *ce)
{
    rp_connector_tcp_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_connector_tcp_ext_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_connector_tcp);
    return &resource->zo;
}

static void free_respond_connector_tcp_resource(zend_object *object)
{
    rp_connector_tcp_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_connector_tcp_ext_t);
    releaseResource(resource);
    zend_object_std_dtor(object);
}

static void connect_async_cb(rp_connector_t *connector)
{
    uv_tcp_t *uv_tcp;
    uv_tcp = rp_malloc(sizeof(uv_tcp_t));
    uv_tcp_init(&main_loop, uv_tcp);
    rp_socket_connect(connector, uv_tcp, &connector->addr);
    fprintf(stderr, "connect: %p %p\n", connector, uv_tcp);
}

PHP_METHOD(respond_connector_tcp, connect)
{
    zval *self = getThis();
    rp_connector_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connector_tcp_ext_t);
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

    connector->zo = Z_OBJ_P(self);
    Z_ADDREF_P(self);
    rp_reactor_async_init((rp_reactor_async_init_cb) connect_async_cb, connector);
}
