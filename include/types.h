#ifndef _RP_TYPES_H
#define _RP_TYPES_H

#define RP_CONNECTION_CLOSED 1

struct rp_reactor_s;
struct rp_client_s;

typedef enum {ACTOR, WORKER_MANAGER, WORKER, TASK_MANAGER, TASK} rp_task_type_t;
typedef enum {RP_TCP, RP_PIPE, RP_UDP, RP_TASK} rp_reactor_type_t;

typedef void (*rp_accepted_cb)(zend_object *server, struct rp_client_s *client, char *ipc_data, size_t ipc_data_len);

typedef union {
    uv_tcp_t  tcp;
    uv_pipe_t pipe;
    uv_udp_t  udp;
} rp_reactor_handler_u;

typedef union {
    struct sockaddr_in sockaddr;
    struct sockaddr_in6 sockaddr6;
    char *socket_path;
} rp_reactor_addr_u;


typedef struct rp_reactor_s {
    rp_reactor_handler_u   handler;
    rp_reactor_type_t      type;
    rp_reactor_addr_u      addr;
    uv_buf_t               dummy_buf;
    uv_connection_cb       connection_cb;
    rp_accepted_cb         accepted_cb;
    zend_object            *server;
    struct rp_reactor_s    *self;
    struct rp_reactor_s    *next;
    uv_signal_t signal;
} rp_reactor_t;

typedef struct {
    rp_reactor_t *reactor;
    size_t data_len;
    char data[0];
} rp_reactor_ext_t;

typedef struct rp_client_s{
    union stream_u {
        uv_stream_t stream;
        uv_pipe_t pipe;
        uv_tcp_t tcp;        
    } stream;
    zend_object       *connection_zo;
    rp_reactor_t      *reactor;
} rp_client_t;

typedef struct {
    uv_write_t uv_write;
    uv_buf_t buf;
    char data[0];
} rp_write_req_t;

static zend_always_inline rp_write_req_t *rp_make_write_req(char *data, size_t data_len)
{
    rp_write_req_t *req;
    req = emalloc(sizeof(rp_write_req_t) + data_len - 1);
    req->buf.base = &req->data;
    req->buf.len = data_len;
    memcpy(req->buf.base, data, data_len);
    return req;
}

#endif
