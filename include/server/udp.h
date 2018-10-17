#ifndef _RP_SERVER_UDP_H
#define _RP_SERVER_UDP_H
#include "internal/event_emitter.h"

CLASS_ENTRY_FUNCTION_D(respond_server_udp);

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_udp, __construct), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_udp, send), 0)
    ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    event_hook_t event_hook;
    rp_reactor_t *reactor;
    zend_object zo;
} rp_udp_ext_t;

PHP_METHOD(respond_server_udp, close);
PHP_METHOD(respond_server_udp, send);
PHP_METHOD(respond_server_udp, __construct);

TRAIT_PHP_METHOD(respond_server_udp, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_udp, event_emitter);
#endif
