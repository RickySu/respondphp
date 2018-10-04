#ifndef _RP_SERVER_TCP_H
#define _RP_SERVER_TCP_H
#include "internal/event_emitter.h"

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_tcp, __construct), 0)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    event_hook_t event_hook;
    rp_reactor_t *reactor;
    zend_object zo;
} rp_tcp_ext_t;

PHP_METHOD(respond_server_tcp, close);
PHP_METHOD(respond_server_tcp, __construct);

TRAIT_PHP_METHOD(respond_server_tcp, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_tcp, event_emitter);
DECLARE_FUNCTION_ENTRY(respond_server_tcp) =
{
    PHP_ME(respond_server_tcp, __construct, ARGINFO(respond_server_tcp, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_tcp, close, NULL, ZEND_ACC_PUBLIC)
    TRAIT_FUNCTION_ENTRY_ME(respond_server_tcp, event_emitter)
    PHP_FE_END
};
#endif
