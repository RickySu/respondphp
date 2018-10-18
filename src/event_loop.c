#include "respondphp.h"
#include "event_loop.h"

DECLARE_FUNCTION_ENTRY(respond_event_loop) =
{
    PHP_ME(respond_event_loop, create, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(respond_event_loop, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(respond_event_loop, run, ARGINFO(respond_event_loop, run), ZEND_ACC_PUBLIC)
    PHP_ME(respond_event_loop, stop, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(respond_event_loop, alive, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static zend_object *create_respond_event_loop_resource(zend_class_entry *class_type);
static void free_respond_event_loop_resource(zend_object *object);

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
    resource = ALLOC_RESOURCE(rp_event_loop_ext_t, ce);
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
}

PHP_METHOD(respond_event_loop, run)
{
    zval retval;
    int worker_ipc_fd, routine_ipc_fd, worker_data_fd;
    long option = RUN_DEFAULT;
    fcall_info_t callback;
    uv_run_mode mode;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), "|lf", &option, FCI_PARSE_PARAMETERS_CC(callback)) == FAILURE) {
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

    if(rp_reactors_count() > 0) {
        rp_init_routine_manager(&routine_ipc_fd);
        rp_init_worker_manager(&worker_ipc_fd, &worker_data_fd);

        if (worker_ipc_fd < 0 || routine_ipc_fd < 0) {
            return;
        }
    }

    rp_init_reactor(worker_ipc_fd, worker_data_fd, routine_ipc_fd);

    if(callback.fcc.initialized){
        fci_call_function(&callback, &retval, 0, NULL);
    }

    uv_run(&main_loop, mode);
}

PHP_METHOD(respond_event_loop, stop)
{
    uv_stop(&main_loop);
}

PHP_METHOD(respond_event_loop, alive)
{
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
