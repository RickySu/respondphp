#include "respondphp.h"
#include "reactor.h"

static rp_reactor_t *rp_reactor_head = NULL;
static rp_reactor_t *rp_reactor_tail = NULL;
static uv_pipe_t ipc_pipe;

static rp_client_t *rp_accept_client(uv_pipe_t *pipe, rp_reactor_t *reactor)
{
    rp_client_t *client = (rp_client_t*) malloc(sizeof(rp_client_t));

    switch(reactor->type){
        case RP_TCP:
            uv_tcp_init(&main_loop, (uv_tcp_t *) client);
            break;
        case RP_PIPE:
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

static void rp_init_worker_server()
{
    int status = uv_read_start((uv_stream_t*) &ipc_pipe, alloc_buffer, (uv_read_cb) rp_reactor_receive);
}

static void close_cb(uv_handle_t* handle){
    free(handle);
}

static void rp_reactor_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf)
{
    rp_reactor_t *reactor = *((rp_reactor_t **) buf->base);
    rp_client_t *client;
    
    if (!uv_pipe_pending_count(pipe)) {
        fprintf(stderr, "(%d) No pending count\n", getpid());
        return;
    }
    
    fprintf(stderr, "recv actor: %d, %x\n", status, reactor);
    RP_ASSERT(reactor->self == reactor);
    
    if(client = rp_accept_client(pipe, reactor)) {
        reactor->accepted_cb(client);
    }
    
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

int rp_init_reactor(int fd)
{
    rp_reactor_t *reactor;
    uv_loop_init(&main_loop);
    uv_pipe_init(&main_loop, &ipc_pipe, 1);
    int ret = uv_pipe_open(&ipc_pipe, fd);
    switch(rp_get_task_type()) {
        case ACTOR:
            rp_init_actor_server();
            break;
        case WORKER:
            rp_init_worker_server();
            break;            
        default:
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

void rp_reactor_send(rp_reactor_t *reactor, uv_stream_t *client, uv_close_cb *close_cb)
{
    reactor_send_req_t *send_req = malloc(sizeof(reactor_send_req_t));
    send_req->close_cb = close_cb;
    send_req->client = client;
    fprintf(stderr, "send actor: %d, %x, %x\n", reactor->dummy_buf.len, reactor, (void *) reactor->dummy_buf.base);
    uv_write2((uv_write_t *) send_req, (uv_stream_t*) &ipc_pipe, &reactor->dummy_buf, 1, client, (uv_write_cb) write2_cb);
}
