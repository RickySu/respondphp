#ifndef _RP_REACTOR_H
#define _RP_REACTOR_H

typedef struct {
    uv_write_t write_req;
    uv_close_cb *close_cb;
    uv_stream_t *client;
} reactor_send_req_t;

static void write2_cb(reactor_send_req_t *req, int status);
static rp_reactor_t *rp_reactor_fetch_head();
static void rp_init_actor_server();
static void rp_init_worker_server();
static void rp_init_task_server();
static void rp_reactor_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf);
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void close_cb(uv_handle_t *handle);
static rp_client_t *accept_client(uv_pipe_t *pipe, rp_reactor_t *reactor);
static void rp_signal_hup_handler(uv_signal_t* signal, int signum);

rp_reactor_t *rp_reactor_add();

#ifdef HAVE_PR_SET_PDEATHSIG
#define DETTACH_SESSION setsid
extern  uv_signal_t signal_handle;
static zend_always_inline void rp_register_pdeath_sig(uv_loop_t *loop, int signum, uv_signal_cb signal_cb)
{
    uv_signal_init(loop, &signal_handle);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    fprintf(stderr, "init signal: %d\n", getpid());
    uv_signal_start(&signal_handle, signal_cb, signum);
}
#else
#define DETTACH_SESSION()
#define rp_register_pdeath_sig(x, y, z)
#endif

#endif
