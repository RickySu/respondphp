#include "respondphp.h"
#include "system/timer.h"

static void timer_async_cb(rp_timer_t *timer);
static void timer_cb(rp_timer_t* timer);

DECLARE_FUNCTION_ENTRY(respond_system_timer) =
{
    PHP_ME(respond_system_timer, __construct, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
    PHP_ME(respond_system_timer, timeout, ARGINFO(respond_system_timer, timeout), ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_system_timer)
{
    REGISTER_CLASS(respond_system_timer, "Respond\\System\\Timer");
    zend_class_implements(CLASS_ENTRY(respond_system_timer), 1, CLASS_ENTRY(respond_socket_connector_interface));
}

static void timer_cb(rp_timer_t* timer)
{
    zval result;
    ZVAL_LONG(&result, 0);
    rp_resolve_promise(&timer->promise, &result);
    ZVAL_PTR_DTOR(&timer->promise);
    rp_free(timer);
}

static void timer_async_cb(rp_timer_t *timer)
{
    int err;
    uv_timer_init(&main_loop, &timer->handle);
    err = uv_timer_start(&timer->handle, (uv_timer_cb) timer_cb, timer->milliseconds, 0);
    if(err < 0){
        rp_reject_promise_long(&timer->promise, err);
        ZVAL_PTR_DTOR(&timer->promise);
        rp_free(timer);
    }
}

PHP_METHOD(respond_system_timer, __construct)
{

}

PHP_METHOD(respond_system_timer, timeout)
{
    zend_long milliseconds;
    rp_timer_t *timer;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "l", &milliseconds)) {
        return;
    }

    timer = rp_malloc(sizeof(rp_timer_t));
    rp_make_promise_object(&timer->promise);
    RETVAL_ZVAL(&timer->promise, 1, 0);

    if(milliseconds < 0){
        rp_reject_promise_long(&timer->promise, -1);
        ZVAL_PTR_DTOR(&timer->promise);
        rp_free(timer);
        return;
    }

    timer->milliseconds = milliseconds;
    rp_reactor_async_init((rp_reactor_async_init_cb) timer_async_cb, timer);
}
