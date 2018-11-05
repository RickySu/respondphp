#ifndef _RP_INTERNAL_SOCKET_CONNECTOR_H
#define _RP_INTERNAL_SOCKET_CONNECTOR_H

typedef struct {
    uv_connect_t connect_req;
    zval promise;
    zend_object *zo;
} rp_connector_t;

void rp_socket_connect_ex(rp_connector_t *connector, uv_handle_t *handle, zval *self, rp_reactor_addr_t *addr, uv_connect_cb callback);
#define rp_socket_connect(connectot, handle, self, addr) rp_socket_connect_ex(connector, handle, self, addr, NULL)

#endif //_RP_INTERNAL_SOCKET_CONNECTOR_H
