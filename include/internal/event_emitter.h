#ifndef _RP_INTERNAL_EVENT_EMMITER_H
#define _RP_INTERNAL_EVENT_EMMITER_H
#include "interface/event_event_emitter_interface.h"

#define TRAIT_FUNCTION_ENTRY_ME_event_emitter(ce) \
    PHP_ME(ce, on, ARGINFO(ce, on), ZEND_ACC_PUBLIC) \
    PHP_ME(ce, off, ARGINFO(ce, off), ZEND_ACC_PUBLIC) \
    PHP_ME(ce, removeListeners, ARGINFO(ce, removeListeners), ZEND_ACC_PUBLIC) \
    PHP_ME(ce, getListeners, ARGINFO(ce, getListeners), ZEND_ACC_PUBLIC) \
    PHP_ME(ce, emit, ARGINFO(ce, emit), ZEND_ACC_PUBLIC)

#define TRAIT_FUNCTION_ARG_INFO_event_emitter(ce) TRAIT_FUNCTION_ARG_INFO_respond_event_event_emitter_interface(ce)

#define TRAIT_PHP_METHOD_event_emitter(ce) \
    PHP_METHOD(ce, on); \
    PHP_METHOD(ce, off); \
    PHP_METHOD(ce, removeListeners); \
    PHP_METHOD(ce, getListeners); \
    PHP_METHOD(ce, emit)

#define TRAIT_PHP_METHOD_USE_event_emitter(ce) \
    PHP_METHOD(ce, on); \
    PHP_METHOD(ce, off); \
    PHP_METHOD(ce, removeListeners); \
    PHP_METHOD(ce, getListeners); \
    PHP_METHOD(ce, emit)

#define TRAIT_PHP_METHOD_USE_event_emitter(ce, resource_type, resource_field) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, on, resource_type, resource_field) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, off, resource_type, resource_field) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, removeListeners, resource_type, resource_field) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, getListeners, resource_type, resource_field) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, emit, resource_type, resource_field)

TRAIT_PHP_METHOD_DEFINE(event_emitter, on);
TRAIT_PHP_METHOD_DEFINE(event_emitter, off);
TRAIT_PHP_METHOD_DEFINE(event_emitter, removeListeners);
TRAIT_PHP_METHOD_DEFINE(event_emitter, getListeners);
TRAIT_PHP_METHOD_DEFINE(event_emitter, emit);
#endif //_RP_INTERNAL_EVENT_EMMITER_H
