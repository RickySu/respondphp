#include "respondphp.h"
#include "internal/socket_connection.h"

void rp_socket_connection_tcp_getRemoteAddress(uv_tcp_t *handle, zval *address)
{
    struct sockaddr addr;
    int addrlen = INET6_ADDRSTRLEN + 6;
    char addr_str[INET6_ADDRSTRLEN + 6];
    uint16_t port;

    if(uv_tcp_getpeername(handle, &addr, &addrlen)){
        ZVAL_NULL(address);
        return;
    }

    sock_addr(&addr, addr_str, sizeof(addr_str), &port);
    snprintf(&addr_str[strlen(addr_str)], 6, ":%d", (int)port);
    ZVAL_STRING(address, addr_str);
}

void rp_socket_connection_tcp_getLocalAddress(uv_tcp_t *handle, zval *address)
{
    struct sockaddr addr;
    int addrlen = INET6_ADDRSTRLEN + 6;
    char addr_str[INET6_ADDRSTRLEN + 6];
    uint16_t port;

    if(uv_tcp_getsockname(handle, &addr, &addrlen)){
        ZVAL_NULL(address);
        return;
    }

    sock_addr(&addr, addr_str, sizeof(addr_str), &port);
    snprintf(&addr_str[strlen(addr_str)], 6, ":%d", (int)port);
    ZVAL_STRING(address, addr_str);
}
