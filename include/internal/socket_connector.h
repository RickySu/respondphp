#ifndef _RP_INTERNAL_SOCKET_CONNECTOR_H
#define _RP_INTERNAL_SOCKET_CONNECTOR_H

typedef struct {
    uv_connect_t connect_req;
    rp_reactor_addr_t addr;
    zval promise;
} rp_connector_t;

void rp_socket_connect_ex(rp_connector_t *connector, uv_handle_t *handle, rp_reactor_addr_t *addr, uv_connect_cb callback);
#define rp_socket_connect(connectot, handle, addr) rp_socket_connect_ex(connector, handle, addr, NULL)

#endif //_RP_INTERNAL_SOCKET_CONNECTOR_H
