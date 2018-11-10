#ifndef RP_SYSTEM_TIMER_PROMISE_H
#define RP_SYSTEM_TIMER_PROMISE_H
#include "interface/async_promise_interface.h"

typedef void (*rp_timer_promise_cancel_cb)(void *data);

CLASS_ENTRY_FUNCTION_D(respond_system_timer_promise);

typedef struct {
    zval promise;
    struct {
        rp_timer_promise_cancel_cb cb;
        void *data;
    } cancel;
    zend_object zo;
} rp_system_timer_promise_t;

PHP_METHOD(respond_system_timer_promise, __construct);
PHP_METHOD(respond_system_timer_promise, then);
PHP_METHOD(respond_system_timer_promise, catch);
PHP_METHOD(respond_system_timer_promise, finally);
PHP_METHOD(respond_system_timer_promise, resolve);
PHP_METHOD(respond_system_timer_promise, reject);
PHP_METHOD(respond_system_timer_promise, cancel);
PHP_METHOD(respond_system_timer_promise, all);
PHP_METHOD(respond_system_timer_promise, race);

TRAIT_FUNCTION_ARG_INFO(respond_system_timer_promise, respond_async_promise);
#endif //RP_SYSTEM_TIMER_PROMISE_H
