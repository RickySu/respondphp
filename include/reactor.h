#ifndef _RP_REACTOR_H
#define _RP_REACTOR_H

typedef struct {
    uv_write_t write_req;
    uv_close_cb close_cb;
    uv_stream_t *client;
    uv_buf_t buf;
    rp_reactor_ext_t reactor_ext;
} reactor_ipc_send_req_t;

typedef struct {
    uv_write_t write_req;
    uv_close_cb close_cb;
    uv_buf_t buf;
    rp_reactor_ext_t reactor_ext;
} reactor_data_send_req_t;

typedef struct {
    rp_reactor_async_init_cb callback;
    void *data
} async_init_t;

#endif
