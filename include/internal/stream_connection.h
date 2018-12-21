#ifndef _RP_INTERNAL_SOCKET_CONNECTION_H
#define _RP_INTERNAL_SOCKET_CONNECTION_H
#include "interface/stream_connection_interface.h"

void rp_stream_connection_tcp_get_remote_address(uv_tcp_t *handle, zval *z_address, zval *z_port);
void rp_stream_connection_tcp_get_local_address(uv_tcp_t *handle, zval *z_address, zval *z_port);

#define TRAIT_FUNCTION_ENTRY_ME_stream_connection(ce) \
    PHP_ME(ce, getRemoteAddress, ARGINFO(ce, getRemoteAddress), ZEND_ACC_PUBLIC) \
    PHP_ME(ce, getLocalAddress, ARGINFO(ce, getLocalAddress), ZEND_ACC_PUBLIC)

#define TRAIT_FUNCTION_ARG_INFO_stream_connection(ce) TRAIT_FUNCTION_ARG_INFO_respond_stream_connection_interface(ce)

#define TRAIT_PHP_METHOD_stream_connection(ce) \
    PHP_METHOD(ce, getRemoteAddress); \
    PHP_METHOD(ce, getLocalAddress)

#define TRAIT_PHP_METHOD_USE_stream_connection(ce) \
    PHP_METHOD(ce, getRemoteAddress); \
    PHP_METHOD(ce, getLocalAddress)

TRAIT_PHP_METHOD_DEFINE(stream_connection, getRemoteAddress);
TRAIT_PHP_METHOD_DEFINE(stream_connection, getLocalAddress);
#endif //_RP_INTERNAL_SOCKET_CONNECTION_H
