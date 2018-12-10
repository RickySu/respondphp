#ifndef RP_SYSTEM_TIMER_H
#define RP_SYSTEM_TIMER_H

CLASS_ENTRY_FUNCTION_D(respond_system_timer);

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_system_timer, timeout), Respond\\Async\\PromiseInterface, 0)
    ZEND_ARG_TYPE_INFO(0, milliseconds, IS_LONG, 0)
ZEND_END_ARG_INFO()

typedef struct {
    uv_timer_t handle;
    uint64_t milliseconds;
    zval promise;
} rp_timer_t;

PHP_METHOD(respond_system_timer, timeout);
PHP_METHOD(respond_system_timer, __construct);
#endif //RP_SYSTEM_TIMER_H
