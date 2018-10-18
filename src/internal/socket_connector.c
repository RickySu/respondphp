#include "respondphp.h"
#include "internal/socket_connector.h"

static void connection_cb(rp_connector_t *connector, int status);

rp_connector_t *rp_socket_connect(uv_handle_t *handle, zval *self, rp_reactor_addr_t *addr)
{
    rp_connector_t *connector;
    connector = rp_malloc(sizeof(rp_connector_t));
    rp_make_promise_object(&connector->promise);
    connector->zo = Z_OBJ_P(self);
    Z_ADDREF_P(self);

    switch (handle->type){
        case UV_TCP:
            uv_tcp_connect(connector, handle, addr, (uv_connect_cb) connection_cb);
            break;
        case UV_NAMED_PIPE:
            uv_pipe_connect(connector, handle, addr, (uv_connect_cb) connection_cb);
            break;
        default:
            break;
    }
    return connector;
}

static void connection_cb(rp_connector_t *connector, int status)
{
    zval result, exception;
    fprintf(stderr, "client connected: %d\n", status);

    if(status < 0) {
        ZVAL_LONG(&result, status);
        object_init_ex(&exception, zend_ce_exception);
        zend_call_method_with_1_params(&exception, NULL, &Z_OBJCE(exception)->constructor, "__construct", NULL, &result);
        rp_reject_promise(&connector->promise, &exception);
        ZVAL_PTR_DTOR(&connector->promise);
        zend_object_ptr_dtor(connector->zo);
        uv_close(connector->connect_req.handle, rp_close_cb_release);
        rp_free(connector);
        return;
    }

    rp_connection_factory(connector->connect_req.handle, &result);
    rp_resolve_promise(&connector->promise, &result);
    ZVAL_PTR_DTOR(&connector->promise);
    zend_object_ptr_dtor(connector->zo);
    rp_free(connector);
}
