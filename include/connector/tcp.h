#ifndef _RP_CONNECTOR_TCP_H
#define _RP_CONNECTOR_TCP_H

CLASS_ENTRY_FUNCTION_D(respond_connector_tcp);

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_connector_tcp, connect), "Respond\\Async\\PromiseInterface", 0)
    ZEND_ARG_VARIADIC_INFO(0, arguments)
ZEND_END_ARG_INFO()

PHP_METHOD(respond_connector_tcp, connect);
PHP_METHOD(respond_connector_tcp, __construct);
#endif
