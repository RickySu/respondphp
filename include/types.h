#ifndef _RP_TYPES_H
#define _RP_TYPES_H

struct rp_reactor_s;
typedef enum {ACTOR, WORKER_MANAGER, WORKER} rp_task_type_t;
typedef enum {RP_TCP, RP_PIPE, RP_UDP} rp_reactor_type_t;

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
    uv_connection_cb       *connection_cb;
    struct rp_reactor_s    *prev;
    struct rp_reactor_s    *next;
} rp_reactor_t;
#endif