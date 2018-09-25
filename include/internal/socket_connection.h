#ifndef _RP_INTERNAL_SOCKET_CONNECTION_H
#define _RP_INTERNAL_SOCKET_CONNECTION_H
#include "interface/socket_connection_interface.h"

static zend_always_inline void sock_addr(struct sockaddr *addr, char *ip_name, size_t ip_len, u_int16_t *port) {
    struct sockaddr_in addr_in;
    struct sockaddr_in6 addr_in6;

    if(addr->sa_family == AF_INET) {
        addr_in = *((struct sockaddr_in *) addr);
        *port = ntohs(addr_in.sin_port);
        memcpy(&addr_in, addr, sizeof(struct sockaddr));
        uv_ip4_name(&addr_in, ip_name, ip_len);
        return;
    }

    addr_in6 = *((struct sockaddr_in6 *) addr);
    *port = ntohs(addr_in6.sin6_port);
    memcpy(&addr_in6, addr, sizeof(struct sockaddr));
    uv_ip6_name(&addr_in6, ip_name, ip_len);
}

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
