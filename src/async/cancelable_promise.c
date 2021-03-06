#include "respondphp.h"
#include "async/cancelable_promise.h"

static zend_object *create_respond_async_cancelable_promise_resource(zend_class_entry *class_type);
static void free_respond_async_cancelable_promise_resource(zend_object *object);
static void forward_call(zval *self, const char *fn, int nargs, zval *args);

DECLARE_FUNCTION_ENTRY(respond_async_cancelable_promise) =
{
    PHP_ME(respond_async_cancelable_promise, __construct, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
    PHP_ME(respond_async_cancelable_promise, then, ARGINFO(respond_async_promise_interface, then), ZEND_ACC_PUBLIC)
    PHP_ME(respond_async_cancelable_promise, catch, ARGINFO(respond_async_promise_interface, catch), ZEND_ACC_PUBLIC)
    PHP_ME(respond_async_cancelable_promise, finally, ARGINFO(respond_async_promise_interface, finally), ZEND_ACC_PUBLIC)
    PHP_ME(respond_async_cancelable_promise, resolve, ARGINFO(respond_async_promise_interface, resolve), ZEND_ACC_PUBLIC)
    PHP_ME(respond_async_cancelable_promise, reject, ARGINFO(respond_async_promise_interface, reject), ZEND_ACC_PUBLIC)
    PHP_ME(respond_async_cancelable_promise, cancel, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_async_cancelable_promise)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_async_cancelable_promise, "Respond\\Async\\CancelablePromise", create_respond_async_cancelable_promise_resource);
    OBJECT_HANDLER(respond_async_cancelable_promise).offset = XtOffsetOf(rp_async_cancelable_promise_t, zo);
    OBJECT_HANDLER(respond_async_cancelable_promise).clone_obj = NULL;
    OBJECT_HANDLER(respond_async_cancelable_promise).free_obj = free_respond_async_cancelable_promise_resource;
    zend_class_implements(CLASS_ENTRY(respond_async_cancelable_promise), 1, CLASS_ENTRY(respond_async_promise_interface));
    zend_declare_property_null(CLASS_ENTRY(respond_async_cancelable_promise), ZEND_STRL("promise"), ZEND_ACC_PRIVATE);
}

static void forward_call(zval *self, const char *fn, int nargs, zval *args)
{
    zval retval, function;
    rp_async_cancelable_promise_t *resource;
    resource = FETCH_OBJECT_RESOURCE(self, rp_async_cancelable_promise_t);
    ZVAL_STRING(&function, fn);
    call_user_function(NULL, &resource->promise, &function, &retval, nargs, args);
    ZVAL_PTR_DTOR(&function);
    ZVAL_PTR_DTOR(&retval);
}

static zend_object *create_respond_async_cancelable_promise_resource(zend_class_entry *ce)
{
    zval thisObject;
    rp_async_cancelable_promise_t *resource;
    resource = ALLOC_RESOURCE(rp_async_cancelable_promise_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_async_cancelable_promise);
    rp_make_promise_object(&resource->promise);
    ZVAL_OBJ(&thisObject, &resource->zo);
    zend_update_property(CLASS_ENTRY(respond_async_cancelable_promise), &thisObject, ZEND_STRL("promise"), &resource->promise);
    ZVAL_PTR_DTOR(&resource->promise);
    RP_ASSERT(Z_REFCOUNT_P(&resource->promise) == 1);
    return &resource->zo;
}

static void free_respond_async_cancelable_promise_resource(zend_object *object)
{
    rp_async_cancelable_promise_t *resource;
    resource = FETCH_RESOURCE(object, rp_async_cancelable_promise_t);
    zend_object_std_dtor(object);
}

PHP_METHOD(respond_async_cancelable_promise, __construct)
{
}

PHP_METHOD(respond_async_cancelable_promise, then)
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

PHP_METHOD(respond_async_cancelable_promise, catch)
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

PHP_METHOD(respond_async_cancelable_promise, finally)
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

PHP_METHOD(respond_async_cancelable_promise, resolve)
{
    zval *args = NULL;
    int nargs = 0;
    zval *self = getThis();

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &nargs)) {
        return;
    }

    forward_call(self, "resolve", nargs, args);
}

PHP_METHOD(respond_async_cancelable_promise, reject)
{
    zval *args = NULL;
    int nargs = 0;
    zval *self = getThis();

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "*", &args, &nargs)) {
        return;
    }

    forward_call(self, "reject", nargs, args);
}

PHP_METHOD(respond_async_cancelable_promise, cancel)
{
    zval *self = getThis();
    rp_async_cancelable_promise_t *resource = FETCH_OBJECT_RESOURCE(self, rp_async_cancelable_promise_t);
    if(resource->cancel.cb != NULL){
        resource->cancel.cb(resource->cancel.data);
    }
}
