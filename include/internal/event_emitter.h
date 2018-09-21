#ifndef _RP_INTERNAL_EVENT_EMMITER_H
#define _RP_INTERNAL_EVENT_EMMITER_H
#include "interface/event_event_emitter_interface.h"

typedef struct {
    HashTable ht;
    int n;
} ht_counter_t;

typedef struct {
    HashTable hook_cache;
    HashTable internal_hook;
    zval hook;
} event_hook_t;

void rp_event_emitter_on(event_hook_t *event_hook, const char *event, size_t event_len, zval *hook);
void rp_event_emitter_off(event_hook_t *event_hook, const char *event, size_t event_len, zval *hook);
void rp_event_emitter_removeListeners(event_hook_t *event_hook, const char *event, size_t event_len);
zval *rp_event_emitter_getListeners(event_hook_t *event_hook, const char *event, size_t event_len);
void rp_event_emitter_emit(event_hook_t *event_hook, const char *event, size_t event_len, zval *param);

void rp_event_hook_init(event_hook_t *hook);
void rp_event_hook_destroy(event_hook_t *hook);
static void rp_event_hook_cache_list_free(zval *hook);
static void rp_event_hook_cache_free(zval *hook);

#define rp_event_hook_clean(hook) \
do{ \
    rp_event_hook_destroy(hook); \
    rp_event_hook_init(hook); \
} while(0)

static void rp_event_hook_cache_free(zval *hook);

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

TRAIT_PHP_METHOD_DEFINE(event_emitter, on);
TRAIT_PHP_METHOD_DEFINE(event_emitter, off);
TRAIT_PHP_METHOD_DEFINE(event_emitter, removeListeners);
TRAIT_PHP_METHOD_DEFINE(event_emitter, getListeners);
TRAIT_PHP_METHOD_DEFINE(event_emitter, emit);
#endif //_RP_INTERNAL_EVENT_EMMITER_H
