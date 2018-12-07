#ifndef _RP_SERVER_TCP_H
#define _RP_SERVER_TCP_H
#include "internal/event_emitter.h"

CLASS_ENTRY_FUNCTION_D(respond_server_tcp);

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_tcp, __construct), 0)
    ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    rp_reactor_t *reactor;
    event_hook_t event_hook;
    zend_object zo;
} rp_server_tcp_ext_t;

PHP_METHOD(respond_server_tcp, close);
PHP_METHOD(respond_server_tcp, __construct);

TRAIT_PHP_METHOD(respond_server_tcp, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_tcp, event_emitter);
#endif
