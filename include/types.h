#ifndef _RP_TYPES_H
#define _RP_TYPES_H

#define RP_CONNECTION_CLOSED 1
#define RP_CONNECTION_SHUTDOWN (1<<1)
#define RP_CONNECTION_RELEASED (1<<2)

struct rp_reactor_s;
struct rp_stream_s;
struct rp_reactor_s;

typedef enum {ACTOR, WORKER_MANAGER, WORKER, ROUTINE_MANAGER, ROUTINE} rp_task_type_t;
typedef enum {RP_TCP, RP_PIPE, RP_UDP, RP_ROUTINE} rp_reactor_type_t;
typedef enum {RP_RECV, RP_SEND} rp_reactor_data_send_type_t;
typedef void (*rp_send_cb)(struct rp_reactor_s *reactor, const char *data, size_t data_len, const struct sockaddr *addr);
typedef void (*rp_data_recv_cb)(zend_object *server, const char *data, size_t data_len, const struct sockaddr *addr, unsigned flags);
typedef void (*rp_accepted_cb)(zend_object *server, struct rp_stream_s *client, char *ipc_data, size_t ipc_data_len);
typedef uv_connection_cb rp_connection_cb;
typedef uv_udp_recv_cb rp_recv_cb;


typedef union {
    uv_tcp_t  tcp;
    uv_pipe_t pipe;
    uv_udp_t  udp;
} rp_reactor_handler_t;

typedef union {
    struct sockaddr_in sockaddr;
    struct sockaddr_in6 sockaddr6;
    const char *socket_path;
} rp_reactor_addr_t;

typedef struct {
    rp_recv_cb             recv;
    rp_data_recv_cb        data_recv;
    rp_send_cb             send;
} rp_dgram_cb_t;

typedef struct {
    rp_connection_cb       connection;
    rp_accepted_cb         accepted;
} rp_stream_cb_t;

typedef union {
    rp_stream_cb_t         stream;
    rp_dgram_cb_t          dgram;
} rp_reactor_cb_t;

typedef struct rp_reactor_s {
    rp_reactor_handler_t   handler;
    rp_reactor_type_t      type;
    rp_reactor_addr_t      addr;
    rp_reactor_cb_t        cb;
    zend_object            *server;
    struct rp_reactor_s    *next;
} rp_reactor_t;

typedef struct {
    rp_reactor_t *reactor;
    size_t data_len;
    char data[0];
} rp_reactor_ext_t;

typedef struct rp_stream_s{
    union stream_u {
        uv_stream_t stream;
        uv_pipe_t pipe;
        uv_tcp_t tcp;
    } stream;
    zend_object       *connection_zo;
} rp_stream_t;

typedef struct {
    union {
        uv_write_t uv_write;
        uv_udp_send_t uv_send;
    } req;
    uv_buf_t buf;
    char data[0];
} rp_write_req_t;

typedef struct {
    struct sockaddr addr;
    unsigned flags;
    size_t data_len;
    char data[0];
} rp_reactor_data_send_req_payload_recv_t;

typedef struct {
    rp_reactor_addr_t addr;
    size_t data_len;
    char data[0];
} rp_reactor_data_send_req_payload_send_t;

typedef struct {
    rp_reactor_data_send_type_t type;
    union {
        rp_reactor_data_send_req_payload_recv_t recv;
        rp_reactor_data_send_req_payload_send_t send;
    } payload;
} rp_reactor_data_send_req_t;
#endif
