#ifndef _RP_ASYNC_PROMISE_INTERFACE_H
#define _RP_ASYNC_PROMISE_INTERFACE_H

#define TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_then(ce) \
RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ARGINFO(ce, then), 0, 0, "Respond\\Async\\PromiseInterface", 0) \
    ZEND_ARG_TYPE_INFO(0, onFullfiled, IS_CALLABLE, 1) \ 
    ZEND_ARG_TYPE_INFO(0, onRejcted, IS_CALLABLE, 1) \
    ZEND_ARG_TYPE_INFO(0, onFinaled, IS_CALLABLE, 1) \
ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_catch(ce) \
RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(ce, catch), "Respond\\Async\\PromiseInterface", 0) \
    ZEND_ARG_TYPE_INFO(0, onRejcted, IS_CALLABLE, 0) \
ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_finally(ce) \
RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(ce, finally), "Respond\\Async\\PromiseInterface", 0) \
    ZEND_ARG_TYPE_INFO(0, onFinal, IS_CALLABLE, 0) \
ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_resolve(ce) \
ZEND_BEGIN_ARG_INFO_EX(ARGINFO(ce, resolve), 0, 0, 0) \
    ZEND_ARG_INFO(0, value) \
ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_reject(ce) \
ZEND_BEGIN_ARG_INFO(ARGINFO(ce, reject), 0) \
    ZEND_ARG_OBJ_INFO(0, reason, Throwable, 0) \
ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_all(ce) \
RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(ce, all), "Respond\\Async\\PromiseInterface", 0) \
    ZEND_ARG_TYPE_INFO(0, promises, IS_ARRAY, 0) \
ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_race(ce) \
RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(ce, race), "Respond\\Async\\PromiseInterface", 0) \
    ZEND_ARG_TYPE_INFO(0, promises, IS_ARRAY, 0) \
ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface(ce) \
    TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_then(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_catch(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_finally(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_resolve(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_reject(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_all(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_async_promise_interface_race(ce)

TRAIT_FUNCTION_ARG_INFO(respond_async_promise_interface, respond_async_promise_interface);
#endif //_RP_ASYNC_PROMISE_INTERFACE_H
