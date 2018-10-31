#ifndef _RP_SERVER_SECURE_H
#define _RP_SERVER_SECURE_H
#ifdef HAVE_OPENSSL
#include "internal/event_emitter.h"

CLASS_ENTRY_FUNCTION_D(respond_server_secure);

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_secure, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, socket, Respond\\Stream\\ServerInterface, 0)
    ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    zend_object *socket_zo;
    HashTable ssl_ctx_ht;
    SSL_CTX *ctx;
    event_hook_t event_hook;
    zend_object zo;
} rp_server_secure_ext_t;

PHP_METHOD(respond_server_secure, close);
PHP_METHOD(respond_server_secure, __construct);

TRAIT_PHP_METHOD(respond_server_secure, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_secure, event_emitter);
#endif
#endif
