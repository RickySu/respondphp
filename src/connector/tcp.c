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
static void client_connection_cb(rp_connector_t* connector, int status);

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

PHP_METHOD(respond_connector_tcp, connect)
{
    zval *self = getThis();
    rp_connector_tcp_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connector_tcp_ext_t);
    zend_string *host;
    zend_long port, ret;
    rp_connector_t *connector;
    rp_reactor_addr_t addr;
    uv_tcp_t *uv_tcp;

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

    uv_tcp = rp_malloc(sizeof(uv_tcp_t));
    uv_tcp_init(&main_loop, uv_tcp);
    connector = rp_socket_connect(uv_tcp, self, &addr);
    fprintf(stderr, "connect: %p %p\n", connector, uv_tcp);
    RETVAL_ZVAL(&connector->promise, 1, 0);
}