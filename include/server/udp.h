#ifndef _RP_SERVER_UDP_H
#define _RP_SERVER_UDP_H
#include "internal/event_emitter.h"

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_udp, __construct), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    event_hook_t event_hook;
    rp_reactor_t *reactor;
    zend_object zo;
} rp_udp_ext_t;

PHP_METHOD(respond_server_udp, close);
PHP_METHOD(respond_server_udp, __construct);

TRAIT_PHP_METHOD(respond_server_udp, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_udp, event_emitter);
DECLARE_FUNCTION_ENTRY(respond_server_udp) =
{
    PHP_ME(respond_server_udp, __construct, ARGINFO(respond_server_udp, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_udp, close, NULL, ZEND_ACC_PUBLIC)
    TRAIT_FUNCTION_ENTRY_ME(respond_server_udp, event_emitter)
    PHP_FE_END
};
#endif
