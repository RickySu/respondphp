#ifndef _RP_INTERNAL_SOCKET_CONNECTION_H
#define _RP_INTERNAL_SOCKET_CONNECTION_H
#include "interface/socket_connection_interface.h"

void rp_socket_connection_tcp_getRemoteAddress(uv_tcp_t *handle, zval *address);
void rp_socket_connection_tcp_getLocalAddress(uv_tcp_t *handle, zval *address);

#define TRAIT_FUNCTION_ENTRY_ME_socket_connection(ce) \
    PHP_ME(ce, getRemoteAddress, ARGINFO(ce, getRemoteAddress), ZEND_ACC_PUBLIC) \
    PHP_ME(ce, getLocalAddress, ARGINFO(ce, getLocalAddress), ZEND_ACC_PUBLIC)

#define TRAIT_FUNCTION_ARG_INFO_socket_connection(ce) TRAIT_FUNCTION_ARG_INFO_respond_socket_connection_interface(ce)

#define TRAIT_PHP_METHOD_socket_connection(ce) \
    PHP_METHOD(ce, getRemoteAddress); \
    PHP_METHOD(ce, getLocalAddress)

#define TRAIT_PHP_METHOD_USE_socket_connection(ce) \
    PHP_METHOD(ce, getRemoteAddress); \
    PHP_METHOD(ce, getLocalAddress)

TRAIT_PHP_METHOD_DEFINE(socket_connection, getRemoteAddress);
TRAIT_PHP_METHOD_DEFINE(socket_connection, getLocalAddress);
#endif //_RP_INTERNAL_SOCKET_CONNECTION_H
