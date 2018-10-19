#ifndef _RP_SERVER_SECURE_H
#define _RP_SERVER_SECURE_H
#ifdef HAVE_OPENSSL

#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include "internal/event_emitter.h"

CLASS_ENTRY_FUNCTION_D(respond_server_secure);

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_secure, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, socket, Respond\\Stream\\ServerInterface, 0)
    ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    event_hook_t event_hook;
    rp_reactor_t *reactor;
    zend_object *socket_zo;
    zend_object zo;
} rp_server_secure_ext_t;

PHP_METHOD(respond_server_secure, close);
PHP_METHOD(respond_server_secure, __construct);

TRAIT_PHP_METHOD(respond_server_secure, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_secure, event_emitter);

#endif
#endif
