#ifndef _RP_CONNECTOR_SECURE_H
#define _RP_CONNECTOR_SECURE_H

CLASS_ENTRY_FUNCTION_D(respond_connector_secure);

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_connector_secure, connect), PREDEFINED_PHP_Respond_Async_PromiseInterface, 0)
    ZEND_ARG_VARIADIC_INFO(0, arguments)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    zend_object zo;
} rp_connector_secure_ext_t;

PHP_METHOD(respond_connector_secure, connect);
#endif
