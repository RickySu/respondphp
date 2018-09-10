#ifndef _RP_EVENT_LOOP_H
#define _RP_EVENT_LOOP_H

#define RUN_DEFAULT  0
#define RUN_ONCE  1
#define RUN_NOWAIT 2

typedef struct {
    zend_object zo;  
} rp_event_loop_ext_t;

CLASS_ENTRY_FUNCTION_D(respond_event_loop);

static zend_object *create_respond_event_loop_resource(zend_class_entry *class_type);
static void free_respond_event_loop_resource(zend_object *object);

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_event_loop, run), 0)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

PHP_METHOD(respond_event_loop, run);
PHP_METHOD(respond_event_loop, stop);
PHP_METHOD(respond_event_loop, alive);
PHP_METHOD(respond_event_loop, __construct);
PHP_METHOD(respond_event_loop, create);

DECLARE_FUNCTION_ENTRY(respond_event_loop) = {
    PHP_ME(respond_event_loop, create, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(respond_event_loop, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(respond_event_loop, run, ARGINFO(respond_event_loop, run), ZEND_ACC_PUBLIC)
    PHP_ME(respond_event_loop, stop, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(respond_event_loop, alive, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

#endif
