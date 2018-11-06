#ifndef _RP_NETWORK_RESOLVER_H
#define _RP_NETWORK_RESOLVER_H

CLASS_ENTRY_FUNCTION_D(respond_network_resolver);

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_network_resolver, getaddrinfo), PREDEFINED_PHP_Respond_Async_PromiseInterface, 0)
    ZEND_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
ZEND_END_ARG_INFO()

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_network_resolver, getnameinfo), PREDEFINED_PHP_Respond_Async_PromiseInterface, 0)
    ZEND_ARG_TYPE_INFO(0, ip_address, IS_STRING, 0)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    zend_object zo;
} rp_network_resolver_ext_t;

typedef struct {
    uv_getaddrinfo_t addrinfo;
    struct addrinfo hints;
    zval promise;
    zend_object *zo;
} rp_getaddrinto_ext_t;

PHP_METHOD(respond_network_resolver, getaddrinfo);
PHP_METHOD(respond_network_resolver, getnameinfo);
#endif //_RP_NETWORK_RESOLVER_H
