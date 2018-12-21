#include "respondphp.h"
#include "internal/stream_connection.h"

void rp_stream_connection_tcp_get_remote_address(uv_tcp_t *handle, zval *z_address, zval *z_port)
{
    struct sockaddr addr;
    int addrlen = INET6_ADDRSTRLEN + 6;
    char addr_str[INET6_ADDRSTRLEN + 6];
    uint16_t port;

    if(uv_tcp_getpeername(handle, &addr, &addrlen)){
        ZVAL_NULL(z_address);
        return;
    }

    sock_addr(&addr, addr_str, sizeof(addr_str), &port);
    if(z_port == NULL) {
        snprintf(&addr_str[strlen(addr_str)], 6, ":%d", (int) port);
    }
    else{
        ZVAL_LONG(&z_port, port);
    }
    ZVAL_STRING(z_address, addr_str);
}

void rp_stream_connection_tcp_get_local_address(uv_tcp_t *handle, zval *z_address, zval *z_port)
{
    struct sockaddr addr;
    int addrlen = INET6_ADDRSTRLEN + 6;
    char addr_str[INET6_ADDRSTRLEN + 6];
    uint16_t port;

    if(uv_tcp_getsockname(handle, &addr, &addrlen)){
        ZVAL_NULL(z_address);
        return;
    }

    sock_addr(&addr, addr_str, sizeof(addr_str), &port);

    if(z_port == NULL) {
        snprintf(&addr_str[strlen(addr_str)], 6, ":%d", (int) port);
    }
    else{
        ZVAL_LONG(&z_port, port);
    }

    ZVAL_STRING(z_address, addr_str);
}
