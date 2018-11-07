#include "respondphp.h"
#include "internal/socket_connector.h"
#include "connector/pipe.h"

DECLARE_FUNCTION_ENTRY(respond_connector_pipe) =
{
    PHP_ME(respond_connector_pipe, connect, ARGINFO(respond_connector_pipe, connect), ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static zend_object *create_respond_connector_pipe_resource(zend_class_entry *class_type);
static void free_respond_connector_pipe_resource(zend_object *object);
static void releaseResource(rp_connector_pipe_ext_t *resource);
static void connect_async_cb(rp_connector_t *connector);

CLASS_ENTRY_FUNCTION_D(respond_connector_pipe)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_connector_pipe, "Respond\\Connector\\Pipe", create_respond_connector_pipe_resource);
    OBJECT_HANDLER(respond_connector_pipe).offset = XtOffsetOf(rp_connector_pipe_ext_t, zo);
    OBJECT_HANDLER(respond_connector_pipe).clone_obj = NULL;
    OBJECT_HANDLER(respond_connector_pipe).free_obj = free_respond_connector_pipe_resource;
    zend_class_implements(CLASS_ENTRY(respond_connector_pipe), 1, CLASS_ENTRY(respond_socket_connector_interface));
}

static void releaseResource(rp_connector_pipe_ext_t *resource)
{
}

static zend_object *create_respond_connector_pipe_resource(zend_class_entry *ce)
{
    rp_connector_pipe_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_connector_pipe_ext_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_connector_pipe);
    return &resource->zo;
}

static void free_respond_connector_pipe_resource(zend_object *object)
{
    rp_connector_pipe_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_connector_pipe_ext_t);
    releaseResource(resource);
    zend_object_std_dtor(object);
}

static void connect_async_cb(rp_connector_t *connector)
{
    uv_pipe_t *uv_pipe;
    uv_pipe = rp_malloc(sizeof(uv_pipe_t));
    uv_pipe_init(&main_loop, uv_pipe, 0);
    rp_socket_connect(connector, uv_pipe, &connector->addr);
    fprintf(stderr, "connect: %p %p\n", connector, uv_pipe);
}

PHP_METHOD(respond_connector_pipe, connect)
{
    zval *self = getThis();
    rp_connector_pipe_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connector_pipe_ext_t);
    rp_connector_t *connector;

    connector = rp_malloc(sizeof(rp_connector_t));
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &connector->addr.socket_path)) {
        rp_free(connector);
        return;
    }

    rp_make_promise_object(&connector->promise);
    connector->zo = Z_OBJ_P(self);
    Z_ADDREF_P(self);
    RETVAL_ZVAL(&connector->promise, 1, 0);
    zend_string_addref(connector->addr.socket_path);
    rp_reactor_async_init((rp_reactor_async_init_cb) connect_async_cb, connector);
}
