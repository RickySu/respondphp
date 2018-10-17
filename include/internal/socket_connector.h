#ifndef _RP_INTERNAL_SOCKET_CONNECTOR_H
#define _RP_INTERNAL_SOCKET_CONNECTOR_H

typedef struct {
    uv_connect_t connect_req;
    zval promise;
    zend_object *zo;
} rp_connector_t;

rp_connector_t *rp_socket_connect(uv_handle_t *handle, zval *self, rp_reactor_addr_t *addr);

#endif //_RP_INTERNAL_SOCKET_CONNECTOR_H
