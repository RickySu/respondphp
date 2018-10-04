#include "respondphp.h"
#include "internal/socket_connection.h"

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

void rp_socket_connection_tcp_getRemoteAddress(uv_tcp_t *handle, zval *address)
{
    struct sockaddr addr;
    int addrlen = sizeof(struct sockaddr_in);
    char addr_str[40];
    uint16_t port;

    if(uv_tcp_getsockname(handle, &addr, &addrlen)){
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
    int addrlen = sizeof(struct sockaddr_in);
    char addr_str[40];
    uint16_t port;

    if(uv_tcp_getpeername(handle, &addr, &addrlen)){
        ZVAL_NULL(address);
        return;
    }

    sock_addr(&addr, addr_str, sizeof(addr_str), &port);
    snprintf(&addr_str[strlen(addr_str)], 6, ":%d", (int)port);
    ZVAL_STRING(address, addr_str);
}
