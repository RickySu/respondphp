#ifndef _RP_EVENT_LOOP_H
#define _RP_EVENT_LOOP_H

#define RUN_DEFAULT  0
#define RUN_ONCE  1
#define RUN_NOWAIT 2

extern ZEND_API zend_class_entry *zend_ce_traversable;
CLASS_ENTRY_FUNCTION_D(respond_event_loop);

typedef struct {
    zend_object zo;  
} rp_event_loop_ext_t;

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_event_loop, run), 0)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

PHP_METHOD(respond_event_loop, run);
PHP_METHOD(respond_event_loop, end);
PHP_METHOD(respond_event_loop, alive);
PHP_METHOD(respond_event_loop, __construct);
PHP_METHOD(respond_event_loop, create);
#endif
