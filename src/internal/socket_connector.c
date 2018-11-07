#include "respondphp.h"
#include "internal/socket_connector.h"

static void connection_cb(rp_connector_t *connector, int status);

void rp_socket_connect_ex(rp_connector_t *connector, uv_handle_t *handle, rp_reactor_addr_t *addr, uv_connect_cb callback)
{
    if(callback == NULL){
        callback = (uv_connect_cb) connection_cb;
    }

    switch (handle->type){
        case UV_TCP:
            uv_tcp_connect(connector, handle, addr, callback);
            break;
        case UV_NAMED_PIPE:
            uv_pipe_connect(connector, handle, addr->socket_path->val, callback);
            zend_string_release(addr->socket_path);
            break;
        default:
            break;
    }
}

static void connection_cb(rp_connector_t *connector, int status)
{
    zval result;
    fprintf(stderr, "client connected: %d\n", status);

    if(status < 0) {
        rp_reject_promise_long(&connector->promise, status);
        ZVAL_PTR_DTOR(&connector->promise);
        zend_object_ptr_dtor(connector->zo);
        uv_close(connector->connect_req.handle, rp_free_cb);
        rp_free(connector);
        return;
    }

    rp_stream_connection_factory(connector->connect_req.handle, &result);
    rp_resolve_promise(&connector->promise, &result);
    ZVAL_PTR_DTOR(&connector->promise);
    zend_object_ptr_dtor(connector->zo);
    rp_free(connector);
}
