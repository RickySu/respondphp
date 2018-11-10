#include "respondphp.h"
#include "system/timer_promise.h"

static zend_object *create_respond_system_timer_promise_resource(zend_class_entry *class_type);
static void free_respond_system_timer_promise_resource(zend_object *object);
static void forward_call(zval *self, const char *fn, int nargs, zval *args);

DECLARE_FUNCTION_ENTRY(respond_system_timer_promise) =
{
    PHP_ME(respond_system_timer_promise, __construct, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
    PHP_ME(respond_system_timer_promise, then, ARGINFO(respond_async_promise_interface, then), ZEND_ACC_PUBLIC)
    PHP_ME(respond_system_timer_promise, catch, ARGINFO(respond_async_promise_interface, catch), ZEND_ACC_PUBLIC)
    PHP_ME(respond_system_timer_promise, finally, ARGINFO(respond_async_promise_interface, finally), ZEND_ACC_PUBLIC)
    PHP_ME(respond_system_timer_promise, resolve, ARGINFO(respond_async_promise_interface, resolve), ZEND_ACC_PUBLIC)
    PHP_ME(respond_system_timer_promise, reject, ARGINFO(respond_async_promise_interface, reject), ZEND_ACC_PUBLIC)
    PHP_ME(respond_system_timer_promise, cancel, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(respond_system_timer_promise, all, ARGINFO(respond_async_promise_interface, all), ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(respond_system_timer_promise, race, ARGINFO(respond_async_promise_interface, race), ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_system_timer_promise)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_system_timer_promise, "Respond\\System\\TimerPromise", create_respond_system_timer_promise_resource);
    OBJECT_HANDLER(respond_system_timer_promise).offset = XtOffsetOf(rp_system_timer_promise_t, zo);
    OBJECT_HANDLER(respond_system_timer_promise).clone_obj = NULL;
    OBJECT_HANDLER(respond_system_timer_promise).free_obj = free_respond_system_timer_promise_resource;
    zend_class_implements(CLASS_ENTRY(respond_system_timer_promise), 1, CLASS_ENTRY(respond_async_promise_interface));
}

static void forward_call(zval *self, const char *fn, int nargs, zval *args)
{
    zval retval, function;
    rp_system_timer_promise_t *resource = FETCH_OBJECT_RESOURCE(self, rp_system_timer_promise_t);
    fprintf(stderr, "call %s with %d args\n", fn, nargs);
    ZVAL_STRING(&function, fn);
    call_user_function(NULL, &resource->promise, &function, &retval, nargs, args);
    fprintf(stderr, "%p %p", Z_STR_P(&function), Z_TYPE_P(&retval) == IS_OBJECT?Z_OBJ_P(&retval):NULL);
    ZVAL_PTR_DTOR(&function);
    ZVAL_PTR_DTOR(&retval);
}

static zend_object *create_respond_system_timer_promise_resource(zend_class_entry *ce)
{
    rp_system_timer_promise_t *resource;
    resource = ALLOC_RESOURCE(rp_system_timer_promise_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_system_timer_promise);
    rp_make_promise_object(&resource->promise);

    fprintf(stderr, "timer promise:%p\n", &resource->zo);
    fprintf(stderr, "promise:%p\n", Z_OBJ_P(&resource->promise));
    return &resource->zo;
}

static void free_respond_system_timer_promise_resource(zend_object *object)
{
    rp_system_timer_promise_t *resource;
    resource = FETCH_RESOURCE(object, rp_system_timer_promise_t);
    ZVAL_PTR_DTOR(&resource->promise);
    zend_object_std_dtor(object);
    fprintf(stderr, "timer promise free\n");
}

PHP_METHOD(respond_system_timer_promise, __construct)
{
}

PHP_METHOD(respond_system_timer_promise, then)
{
    zval *args = NULL;
    int nargs = 0;
    zval *self = getThis();

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &nargs)) {
        return;
    }

    forward_call(self, "then", nargs, args);

    RETURN_ZVAL(self, 1 ,0);
}

PHP_METHOD(respond_system_timer_promise, catch)
{
    zval *args = NULL;
    int nargs = 0;
    zval *self = getThis();

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &nargs)) {
        return;
    }

    forward_call(self, "catch", nargs, args);
    RETURN_ZVAL(getThis(), 1 ,0);
}

PHP_METHOD(respond_system_timer_promise, finally)
{
    zval *args = NULL;
    int nargs = 0;
    zval *self = getThis();

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &nargs)) {
        return;
    }

    forward_call(self, "finally", nargs, args);
    RETURN_ZVAL(self, 1 ,0);
}

PHP_METHOD(respond_system_timer_promise, resolve)
{
    zval *args = NULL;
    int nargs = 0;
    zval *self = getThis();

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &nargs)) {
        return;
    }

    forward_call(self, "resolve", nargs, args);
}

PHP_METHOD(respond_system_timer_promise, reject)
{
    zval *args = NULL;
    int nargs = 0;
    zval *self = getThis();

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &nargs)) {
        return;
    }

    forward_call(self, "reject", nargs, args);
}

PHP_METHOD(respond_system_timer_promise, cancel)
{
    zval *self = getThis();
    rp_system_timer_promise_t *resource = FETCH_OBJECT_RESOURCE(self, rp_system_timer_promise_t);
    if(resource->cancel.cb != NULL){
        resource->cancel.cb(resource->cancel.data);
    }
}

PHP_METHOD(respond_system_timer_promise, all)
{
//    RETURN_ZVAL(getThis(), 1 ,0);
}

PHP_METHOD(respond_system_timer_promise, race)
{
//    RETURN_ZVAL(getThis(), 1 ,0);
}
