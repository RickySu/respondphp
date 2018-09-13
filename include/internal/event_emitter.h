#ifndef _RP_INTERNAL_EVENT_EMMITER_H
#define _RP_INTERNAL_EVENT_EMMITER_H

#define TRAIT_FUNCTION_ENTRY_ME_event_emitter(ce) \
    PHP_ME(ce, on, NULL, ZEND_ACC_PUBLIC) \
    PHP_ME(ce, off, NULL, ZEND_ACC_PUBLIC) \
    PHP_ME(ce, removeListeners, NULL, ZEND_ACC_PUBLIC) \
    PHP_ME(ce, getListeners, NULL, ZEND_ACC_PUBLIC) \
    PHP_ME(ce, emit, NULL, ZEND_ACC_PUBLIC)

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

#define TRAIT_PHP_METHOD_USE_event_emitter(ce) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, on) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, off) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, removeListeners) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, getListeners) \
    TRAIT_PHP_METHOD_PASSTHRU(ce, event_emitter, emit)

TRAIT_PHP_METHOD_DEFINE(event_emitter, on);
TRAIT_PHP_METHOD_DEFINE(event_emitter, off);
TRAIT_PHP_METHOD_DEFINE(event_emitter, removeListeners);
TRAIT_PHP_METHOD_DEFINE(event_emitter, getListeners);
TRAIT_PHP_METHOD_DEFINE(event_emitter, emit);
#endif //_RP_INTERNAL_EVENT_EMMITER_H
