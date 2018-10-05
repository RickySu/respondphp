#include "respondphp.h"
#include "reactor.h"

#ifdef HAVE_PR_SET_PDEATHSIG
uv_signal_t signal_handle;
#endif

static rp_reactor_t *rp_reactor_head = NULL;
static rp_reactor_t *rp_reactor_tail = NULL;

static void write2_cb(reactor_send_req_t *req, int status);
static void rp_init_actor_server();
static int rp_init_worker_server(int fd);
static int rp_init_routine_server(int fd);
static void rp_reactor_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf);
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void close_cb(uv_handle_t *handle);
static rp_client_t *rp_accept_client(uv_pipe_t *pipe, rp_reactor_t *reactor);
static void rp_signal_hup_handler(uv_signal_t* signal, int signum);

static rp_client_t *rp_accept_client(uv_pipe_t *pipe, rp_reactor_t *reactor)
{
    rp_client_t *client = (rp_client_t*) rp_malloc(sizeof(rp_client_t));

    switch(reactor->type){
        case RP_TCP:
            uv_tcp_init(&main_loop, (uv_tcp_t *) client);
            break;
        case RP_PIPE:
        case RP_ROUTINE:
            uv_pipe_init(&main_loop, (uv_pipe_t *) client, 0);
            break;
        default:
            break;
    }
    
    if (uv_accept((uv_stream_t *) pipe, (uv_stream_t*) client) == 0) {
        return client;
    }

    uv_close((uv_handle_t*) client, close_cb);
    rp_free(client);
    return NULL;
}

rp_reactor_t *rp_reactor_get_head()
{
    return rp_reactor_head;
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) rp_malloc(suggested_size);
    buf->len = suggested_size;
}

static int rp_init_worker_server(int fd)
{
    int ret;

    if((ret = uv_pipe_init(&main_loop, &ipc_pipe, 1)) != 0){
        return ret;
    }

    if((ret = uv_pipe_open(&ipc_pipe, fd)) != 0){
        return ret;
    }

    return uv_read_start((uv_stream_t*) &ipc_pipe, alloc_buffer, (uv_read_cb) rp_reactor_receive);
}

static int rp_init_routine_server(int fd)
{
    int ret;

    if((ret = uv_pipe_init(&main_loop, &routine_pipe, 1)) != 0){
        return ret;
    }

    if((ret = uv_pipe_open(&routine_pipe, fd)) != 0){
        return ret;
    }

    return uv_read_start((uv_stream_t*) &routine_pipe, alloc_buffer, (uv_read_cb) rp_reactor_receive);
}

static void close_cb(uv_handle_t* handle)
{
    rp_free(handle);
}

static void rp_reactor_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf)
{
    rp_reactor_ext_t *reactor_ext = (rp_reactor_ext_t *) buf->base;
    rp_client_t *client;

    if (!uv_pipe_pending_count(pipe)) {
        rp_free(buf->base);
        return;
    }

    if((client = rp_accept_client(pipe, reactor_ext->reactor)) != NULL) {
        reactor_ext->reactor->accepted_cb(reactor_ext->reactor->server, client, reactor_ext->data, reactor_ext->data_len);
    }

    rp_free(buf->base);
}

static void write2_cb(reactor_send_req_t *req, int status)
{
    uv_close((uv_handle_t *) req->client, (uv_close_cb) req->close_cb);
    rp_free(req);
}

static void rp_init_actor_server()
{
    rp_reactor_t *reactor = rp_reactor_get_head();

    while(reactor) {
        switch(reactor->type) {
            case RP_TCP:
                uv_tcp_init(&main_loop, &reactor->handler.tcp);
                uv_tcp_bind(&reactor->handler.tcp, (const struct sockaddr*) &reactor->addr, 0);
                uv_listen((uv_stream_t *) &reactor->handler.tcp, SOMAXCONN, reactor->connection_cb);
                break;
            case RP_PIPE:
                uv_pipe_init(&main_loop, &reactor->handler.pipe, 0);
                uv_pipe_bind(&reactor->handler.pipe, reactor->addr.socket_path);
                uv_listen((uv_stream_t *) &reactor->handler.pipe, SOMAXCONN, reactor->connection_cb);
                break;
            default:
                break;
        }
        reactor = reactor->next;        
    }
}

int rp_init_reactor(int worker_fd, int routine_fd)
{
    int ret = 0;
    uv_loop_init(&main_loop);
    switch(rp_get_task_type()) {
        case ACTOR:
            uv_pipe_init(&main_loop, &ipc_pipe, 1);
            ret = uv_pipe_open(&ipc_pipe, worker_fd);
            rp_register_pdeath_sig(&main_loop, SIGINT, rp_signal_hup_handler);
            rp_init_actor_server();
            break;
        case WORKER:
            rp_register_pdeath_sig(&main_loop, SIGHUP, rp_signal_hup_handler);
            ret = rp_init_worker_server(worker_fd);
            uv_pipe_init(&main_loop, &routine_pipe, 1);
            uv_pipe_open(&routine_pipe, routine_fd);
            fprintf(stderr, "worker: %d\n", getpid());
            break;            
        case ROUTINE:
            rp_register_pdeath_sig(&main_loop, SIGHUP, rp_signal_hup_handler);
            ret = rp_init_routine_server(routine_fd);
            break;
        case WORKER_MANAGER:
            break;
        case ROUTINE_MANAGER:
            break;
    }
    return ret;
}

void rp_reactor_destroy()
{
    rp_reactor_t *reactor = rp_reactor_get_head();
    rp_reactor_t *tmp_reactor;
    while(reactor) {
        tmp_reactor = reactor;
        reactor = reactor->next;
        rp_free(tmp_reactor);
    }
}

rp_reactor_t *rp_reactor_add()
{
    rp_reactor_t *reactor = rp_calloc(1, sizeof(rp_reactor_t));

    if(rp_reactor_head == NULL) {
        rp_reactor_head = rp_reactor_tail = reactor;
        return reactor;
    }
    
    rp_reactor_tail->next = reactor;
    rp_reactor_tail = reactor;
    
    return reactor;
}

void rp_reactor_send_ex(rp_reactor_t *reactor, uv_stream_t *client, uv_close_cb close_cb, char *data, size_t data_len, uv_stream_t *ipc)
{
    reactor_send_req_t *send_req = rp_malloc(sizeof(reactor_send_req_t) + data_len - 1);
    send_req->close_cb = close_cb;
    send_req->client = client;
    send_req->reactor_ext.reactor = reactor;
    send_req->reactor_ext.data_len = data_len;
    memcpy(&send_req->reactor_ext.data, data, data_len);
    send_req->buf.base = (char *) &send_req->reactor_ext;
    send_req->buf.len = sizeof(send_req->reactor_ext) + data_len;
    uv_write2((uv_write_t *) send_req, ipc, &send_req->buf, 1, client, (uv_write_cb) write2_cb);
}

static void rp_signal_hup_handler(uv_signal_t* signal, int signum)
{
    uv_signal_stop(signal);
    uv_stop(&main_loop);
}
