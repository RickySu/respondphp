#ifndef _RP_ASYNC_PROMISE_INTERFACE_H
#define _RP_ASYNC_PROMISE_INTERFACE_H

CLASS_ENTRY_FUNCTION_D(respond_async_promise_interface);

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ARGINFO(respond_async_promise_interface, then), 0, 0, "Respond\\Async\\PromiseInterface", 0)
    ZEND_ARG_TYPE_INFO(0, onFullfiled, IS_CALLABLE, 1)
    ZEND_ARG_TYPE_INFO(0, onRejcted, IS_CALLABLE, 1)
    ZEND_ARG_TYPE_INFO(0, onFinaled, IS_CALLABLE, 1)
ZEND_END_ARG_INFO()

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_async_promise_interface, catch), "Respond\\Async\\PromiseInterface", 0)
    ZEND_ARG_TYPE_INFO(0, onRejcted, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_async_promise_interface, finally), "Respond\\Async\\PromiseInterface", 0)
    ZEND_ARG_TYPE_INFO(0, onFinal, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ARGINFO(respond_async_promise_interface, resolve), 0, 0, 0)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_async_promise_interface, reject), 0)
    ZEND_ARG_OBJ_INFO(0, reason, Throwable, 0)
ZEND_END_ARG_INFO()

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_async_promise_interface, all), "Respond\\Async\\PromiseInterface", 0)
    ZEND_ARG_TYPE_INFO(0, promises, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_async_promise_interface, race), "Respond\\Async\\PromiseInterface", 0)
    ZEND_ARG_TYPE_INFO(0, promises, IS_ARRAY, 0)
ZEND_END_ARG_INFO()
#endif //_RP_ASYNC_PROMISE_INTERFACE_H
