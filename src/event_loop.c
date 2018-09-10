#include "respondphp.h"
#include "event_loop.h"

extern ZEND_API zend_class_entry *zend_ce_traversable;
CLASS_ENTRY_FUNCTION_D(respond_event_loop)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_event_loop, "Respond\\Event\\Loop", create_respond_event_loop_resource);
    OBJECT_HANDLER(respond_event_loop).offset = offsetof(rp_event_loop_ext_t, zo);
    OBJECT_HANDLER(respond_event_loop).clone_obj = NULL;
    OBJECT_HANDLER(respond_event_loop).free_obj = free_respond_event_loop_resource;
    REGISTER_CLASS_CONSTANT_LONG(respond_event_loop, RUN_DEFAULT);
    REGISTER_CLASS_CONSTANT_LONG(respond_event_loop, RUN_ONCE);
    REGISTER_CLASS_CONSTANT_LONG(respond_event_loop, RUN_NOWAIT);    
    zend_declare_property_null(CLASS_ENTRY(respond_event_loop), ZEND_STRL("loop"), ZEND_ACC_PRIVATE|ZEND_ACC_STATIC);
}

static zend_object *create_respond_event_loop_resource(zend_class_entry *ce)
{
    rp_event_loop_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_event_loop_ext_t);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_event_loop);
    return &resource->zo;
}

static void free_respond_event_loop_resource(zend_object *object)
{
    rp_event_loop_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_event_loop_ext_t);
    zend_object_std_dtor(&resource->zo);
    efree(resource);
}

PHP_METHOD(respond_event_loop, run)
{
    int fd;
    long option = RUN_DEFAULT;
    uv_run_mode mode;
    zval *self = getThis();
    rp_event_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_event_loop_ext_t);
    if(zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &option) == FAILURE) {
        return;
    }    

    switch(option){
        case RUN_DEFAULT:
            mode = UV_RUN_DEFAULT;
            break;
        case RUN_ONCE:
            mode = UV_RUN_ONCE;
            break;
        case RUN_NOWAIT:
            mode = UV_RUN_NOWAIT;
            break;
        default:
            mode = UV_RUN_DEFAULT;        
    }
    
    fd = rp_init_worker_manager();
    
    if(fd < 0) {
        return;
    }
    
    int ret = rp_init_reactor(fd);
    fprintf(stderr, "init pipe: %d\n", ret);
    uv_run(&main_loop, mode);
}

PHP_METHOD(respond_event_loop, stop)
{
    zval *self = getThis();
    rp_event_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_event_loop_ext_t);
    uv_stop(&main_loop);
}

PHP_METHOD(respond_event_loop, alive)
{
    zval *self = getThis();
    rp_event_loop_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_event_loop_ext_t);
    RETURN_LONG(uv_loop_alive(&main_loop));
}

PHP_METHOD(respond_event_loop, __construct)
{
}

PHP_METHOD(respond_event_loop, create)
{
    zval *instance = zend_read_static_property(CLASS_ENTRY(respond_event_loop), ZEND_STRL("loop"), 1);
    if(Z_ISNULL_P(instance)) {
        object_init_ex(instance, CLASS_ENTRY(respond_event_loop));
        zend_update_static_property(CLASS_ENTRY(respond_event_loop), ZEND_STRL("loop"), instance);
    }
    RETURN_ZVAL(instance, 1, 0);
}
