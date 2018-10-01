#include "respondphp.h"
#include "reactor.h"

static rp_reactor_t *rp_reactor_head = NULL;
static rp_reactor_t *rp_reactor_tail = NULL;

#ifdef HAVE_PR_SET_PDEATHSIG
uv_signal_t signal_handle;
#endif

static rp_client_t *rp_accept_client(uv_pipe_t *pipe, rp_reactor_t *reactor)
{
    rp_client_t *client = (rp_client_t*) malloc(sizeof(rp_client_t));

    switch(reactor->type){
        case RP_TCP:
            uv_tcp_init(&main_loop, (uv_tcp_t *) client);
            break;
        case RP_PIPE:
        case RP_TASK:
            uv_pipe_init(&main_loop, (uv_pipe_t *) client, 0);
            break;
        default:
            break;
    }
    
    if (uv_accept((uv_stream_t *) pipe, (uv_stream_t*) client) == 0) {
        return client;
    }

    uv_close((uv_handle_t*) client, close_cb);
    free(client);
    return NULL;
}

static rp_reactor_t *rp_reactor_get_head()
{
    return rp_reactor_head;
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static int rp_init_worker_server(int fd)
{
    int ret;

    if(ret = uv_pipe_init(&main_loop, &ipc_pipe, 1)){
        return ret;
    }

    if(ret = uv_pipe_open(&ipc_pipe, fd)){
        return ret;
    }

    return uv_read_start((uv_stream_t*) &ipc_pipe, alloc_buffer, (uv_read_cb) rp_reactor_receive);
}

static int rp_init_task_server(int fd)
{
    int ret;

    if(ret = uv_pipe_init(&main_loop, &task_pipe, 1)){
        return ret;
    }

    if(ret = uv_pipe_open(&task_pipe, fd)){
        return ret;
    }

    return uv_read_start((uv_stream_t*) &task_pipe, alloc_buffer, (uv_read_cb) rp_reactor_receive);
}

static void close_cb(uv_handle_t* handle){
    free(handle);
}

static void rp_reactor_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf)
{
    rp_reactor_ext_t *reactor_ext = (rp_reactor_ext_t *) buf->base;
    rp_client_t *client;

    fprintf(stderr, "recv: %p %p\n", reactor_ext, buf->base);

    if (!uv_pipe_pending_count(pipe)) {
        fprintf(stderr, "(%d) No pending count %p\n", getpid(), buf->base);
        free(buf->base);
        return;
    }
    rp_reactor_t *reactor = reactor_ext->reactor;

    fprintf(stderr, "recv actor: %d, %p, %p %d\n", status, reactor_ext, reactor, reactor_ext->data_len);

    if(reactor_ext->data_len > 0) {
        fprintf(stderr, "recv actor data: %.*s\n", reactor_ext->data_len, reactor_ext->data);
    }

    RP_ASSERT(reactor_ext->reactor->self == reactor_ext->reactor);

    fprintf(stderr, "recv accepted_cb: %p\n", reactor_ext->reactor->accepted_cb);

    if(client = rp_accept_client(pipe, reactor_ext->reactor)) {
        reactor_ext->reactor->accepted_cb(reactor_ext->reactor->server, client, reactor_ext->data, reactor_ext->data_len);
    }

    free(buf->base);
}

static void write2_cb(reactor_send_req_t *req, int status)
{
    uv_close(req->client, req->close_cb);
    free(req);
}

static void rp_init_actor_server()
{
    rp_reactor_t *reactor = rp_reactor_get_head();

    while(reactor) {
        fprintf(stderr, "init actor server\n");
        switch(reactor->type) {
            case RP_TCP:
                fprintf(stderr, "init tcp server\n");
                uv_tcp_init(&main_loop, &reactor->handler.tcp);
                uv_tcp_bind(&reactor->handler.tcp, (const struct sockaddr*) &reactor->addr, 0);
                uv_listen(&reactor->handler.tcp, SOMAXCONN, reactor->connection_cb);
                break;
            default:
                break;
        }
        reactor = reactor->next;        
    }
}

int rp_init_reactor(int worker_fd, int task_fd)
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
            fprintf(stderr, "worker: %d\n", getpid());
            rp_register_pdeath_sig(&main_loop, SIGHUP, rp_signal_hup_handler);
            ret = rp_init_worker_server(worker_fd);

            uv_pipe_init(&main_loop, &task_pipe, 1);
            uv_pipe_open(&task_pipe, task_fd);
            break;            
        case TASK:
            fprintf(stderr, "task: %d\n", getpid());
            rp_register_pdeath_sig(&main_loop, SIGHUP, rp_signal_hup_handler);
            ret = rp_init_task_server(task_fd);
            break;
        case WORKER_MANAGER:
            break;
        case TASK_MANAGER:
            break;
    }
    return ret;
}

rp_reactor_t *rp_reactor_add()
{
    rp_reactor_t *reactor = calloc(1, sizeof(rp_reactor_t));
    reactor->self = reactor;
    reactor->dummy_buf = uv_buf_init((char *) &reactor->self, sizeof(reactor));
    fprintf(stderr, "ss: %x %x\n", reactor, (void*) reactor->dummy_buf.base);
    if(rp_reactor_head == NULL){
        rp_reactor_head = rp_reactor_tail = reactor;
        return reactor;
    }
    
    rp_reactor_tail->next = reactor;
    rp_reactor_tail = reactor;
    
    return reactor;
}

void rp_reactor_send_ex(rp_reactor_t *reactor, uv_stream_t *client, uv_close_cb *close_cb, char *data, size_t data_len, uv_stream_t *ipc)
{
    reactor_send_req_t *send_req = malloc(sizeof(reactor_send_req_t) + data_len - 1);
    send_req->close_cb = close_cb;
    send_req->client = client;
    send_req->reactor_ext.reactor = reactor;
    send_req->reactor_ext.data_len = data_len;
    memcpy(&send_req->reactor_ext.data, data, data_len);
    send_req->buf.base = &send_req->reactor_ext;
    send_req->buf.len = sizeof(send_req->reactor_ext) + data_len;
    fprintf(stderr, "send actor len: %d, %d, %d, %d, %d, %d\n",
            send_req->buf.len,
            sizeof(send_req->reactor_ext),
            sizeof(send_req->reactor_ext.reactor),
            sizeof(send_req->reactor_ext.data_len),
            sizeof(send_req->reactor_ext.data),
            send_req->reactor_ext.data_len);
    fprintf(stderr, "send actor: %d, %p, %p %d\n", send_req->buf.len, reactor, (void *) send_req->buf.base, send_req->reactor_ext.data_len);
    int ret = uv_write2((uv_write_t *) send_req, ipc, &send_req->buf, 1, client, (uv_write_cb) write2_cb);
    fprintf(stderr, "send actor end: %d %s %.*s\n", ret, uv_strerror(ret), send_req->reactor_ext.data_len, &send_req->reactor_ext.data);
}

static void rp_signal_hup_handler(uv_signal_t* signal, int signum)
{
    fprintf(stderr, "worker HUP %d %d %d\n", SIGHUP, signum, getpid());
    uv_signal_stop(signal);
    uv_stop(&main_loop);
}
