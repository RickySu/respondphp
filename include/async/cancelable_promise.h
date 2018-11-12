#ifndef RP_ASYNC_CANCELABLE_PROMISE_H
#define RP_ASYNC_CANCELABLE_PROMISE_H
#include "interface/async_promise_interface.h"

typedef void (*rp_timer_promise_cancel_cb)(void *data);

CLASS_ENTRY_FUNCTION_D(respond_async_cancelable_promise);

typedef struct {
    struct {
        rp_timer_promise_cancel_cb cb;
        void *data;
    } cancel;
    zval promise;
    zend_object zo;
} rp_async_cancelable_promise_t;

PHP_METHOD(respond_async_cancelable_promise, __construct);
PHP_METHOD(respond_async_cancelable_promise, then);
PHP_METHOD(respond_async_cancelable_promise, catch);
PHP_METHOD(respond_async_cancelable_promise, finally);
PHP_METHOD(respond_async_cancelable_promise, resolve);
PHP_METHOD(respond_async_cancelable_promise, reject);
PHP_METHOD(respond_async_cancelable_promise, cancel);

TRAIT_FUNCTION_ARG_INFO(respond_async_cancelable_promise, respond_async_promise);
#endif //RP_ASYNC_CANCELABLE_PROMISE_H
