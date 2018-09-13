#ifndef _RP_TYPES_H
#define _RP_TYPES_H

struct rp_reactor_s;
struct rp_client_s;

typedef enum {ACTOR, WORKER_MANAGER, WORKER} rp_task_type_t;
typedef enum {RP_TCP, RP_PIPE, RP_UDP} rp_reactor_type_t;

typedef void (*rp_accepted_cb)(struct rp_client_s *client);

typedef union {
    uv_tcp_t  tcp;
    uv_pipe_t pipe;
    uv_udp_t  udp;
} rp_reactor_handler_u;

typedef union {
    struct sockaddr socket_addr;
    char *socket_path;
} rp_reactor_addr_u;


typedef struct rp_reactor_s {
    rp_reactor_handler_u   handler;
    rp_reactor_type_t      type;
    rp_reactor_addr_u      addr;
    uv_buf_t               dummy_buf;
    uv_connection_cb       connection_cb;
    rp_accepted_cb         accepted_cb;
    struct rp_reactor_s    *self;
    struct rp_reactor_s    *next;
} rp_reactor_t;

typedef struct rp_client_s{
    union stream_u {
        uv_pipe_t pipe;
        uv_tcp_t tcp;        
    } stream;
    rp_reactor_t *reactor;
} rp_client_t;
#endif
