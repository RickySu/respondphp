#ifndef _RP_CONNECTOR_SECURE_H
#define _RP_CONNECTOR_SECURE_H

CLASS_ENTRY_FUNCTION_D(respond_connector_secure);

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_connector_secure, connect), Respond\\Async\\PromiseInterface, 0)
    ZEND_ARG_VARIADIC_INFO(0, arguments)
ZEND_END_ARG_INFO()

typedef struct {
    rp_connector_t connector;
    zend_string *server_name;
    rp_stream_secure_ext_t *connection_secure_resource;
} rp_secure_connector_t;

PHP_METHOD(respond_connector_secure, connect);
PHP_METHOD(respond_connector_secure, __construct);
#endif
