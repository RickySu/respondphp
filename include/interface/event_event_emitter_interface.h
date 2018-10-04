#ifndef _RP_EVENT_EVENT_EMITTER_INTERFACE_H
#define _RP_EVENT_EVENT_EMITTER_INTERFACE_H

#define TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_on(ce) \
    ZEND_BEGIN_ARG_INFO(ARGINFO(ce, on), 0) \
        ZEND_ARG_TYPE_INFO(0, event, IS_STRING, 0) \
        ZEND_ARG_CALLABLE_INFO(0, listener, 0) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_off(ce) \
    ZEND_BEGIN_ARG_INFO(ARGINFO(ce, off), 0) \
        ZEND_ARG_TYPE_INFO(0, event, IS_STRING, 0) \
        ZEND_ARG_CALLABLE_INFO(0, listener, 0) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_removeListeners(ce) \
    ZEND_BEGIN_ARG_INFO(ARGINFO(ce, removeListeners), 0) \
        ZEND_ARG_TYPE_INFO(0, event, IS_STRING, 0) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_getListeners(ce) \
    RP_BEGIN_ARG_INFO(ARGINFO(ce, getListeners), IS_ARRAY, 1) \
        ZEND_ARG_TYPE_INFO(0, event, IS_STRING, 0) \
    RP_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_emit(ce) \
    ZEND_BEGIN_ARG_INFO(ARGINFO(ce, emit), 0) \
        ZEND_ARG_TYPE_INFO(0, event, IS_STRING, 0) \
        ZEND_ARG_VARIADIC_INFO(0, arguments) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface(ce) \
    TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_on(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_off(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_removeListeners(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_getListeners(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface_emit(ce)

TRAIT_FUNCTION_ARG_INFO(respond_event_event_emitter_interface, respond_event_event_emitter_interface);
#endif
