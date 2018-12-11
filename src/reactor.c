#include "respondphp.h"
#include "reactor.h"

#ifdef HAVE_PR_SET_PDEATHSIG
uv_signal_t signal_handle;
#endif

static HashTable rp_reactors;
static HashTable rp_reactors_async_inits;
static void write2_cb(reactor_ipc_send_req_t *req, int status);
static void rp_init_actor_server(int worker_ipc_fd, int worker_data_fd);
static int rp_init_worker_server(int worker_ipc_fd, int worker_data_fd);
static int rp_init_routine_server(int routine_ipc_fd);
static void rp_reactor_ipc_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf);
static void rp_reactor_data_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf);
static rp_stream_t *rp_accept_client(uv_pipe_t *pipe, rp_reactor_t *reactor);
static void rp_signal_hup_handler(uv_signal_t* signal, int signum);
static void reactors_destroy(zval *reactor_p);
static void reactor_async_init_free(zval *data);
static void rp_reactor_actor_ipc_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf);
static void reactor_free(rp_reactor_t *reactor);

static rp_stream_t *rp_accept_client(uv_pipe_t *pipe, rp_reactor_t *reactor)
{
    rp_stream_t *client = (rp_stream_t*) rp_malloc(sizeof(rp_stream_t));

    switch(reactor->type){
        case RP_TCP:
            uv_tcp_init(&main_loop, (uv_tcp_t *) client);
            break;
        case RP_PIPE:
        case RP_ROUTINE:
        case RP_UDP:
            uv_pipe_init(&main_loop, (uv_pipe_t *) client, 0);
            break;
        default:
            break;
    }
    
    if (uv_accept((uv_stream_t *) pipe, (uv_stream_t*) client) == 0) {
        return client;
    }

    uv_close((uv_handle_t*) client, rp_free_cb);
    rp_free(client);
    return NULL;
}

void rp_alloc_buffer_zend_string(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    zend_string *string = zend_string_alloc(suggested_size + 1, 0);
    buf->base = string->val;
    buf->len = suggested_size;
}

void rp_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) rp_malloc(suggested_size);
    buf->len = suggested_size;
}

static int rp_init_routine_server(int routine_ipc_fd)
{
    int ret;

    if((ret = uv_pipe_init(&main_loop, &routine_pipe, 1)) != 0){
        return ret;
    }

    if((ret = uv_pipe_open(&routine_pipe, routine_ipc_fd)) != 0){
        return ret;
    }

    return uv_read_start((uv_stream_t*) &routine_pipe, rp_alloc_buffer, (uv_read_cb) rp_reactor_ipc_receive);
}

static void rp_reactor_ipc_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf)
{
    rp_reactor_ext_t *reactor_ext = (rp_reactor_ext_t *) buf->base;
    rp_stream_t *client;

    if (!uv_pipe_pending_count(pipe)) {
        rp_free(buf->base);
        return;
    }

    fprintf(stderr, "reactor ipc recv: %d %p\n", status, reactor_ext->reactor);

    if((client = rp_accept_client(pipe, reactor_ext->reactor)) != NULL) {
        reactor_ext->reactor->cb.stream.accepted(reactor_ext->reactor->server, client, reactor_ext->data, reactor_ext->data_len);
    }

    rp_free(buf->base);
}

static void rp_reactor_actor_ipc_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf)
{
    rp_reactor_ext_t *reactor_ext = (rp_reactor_ext_t *) buf->base;

    if (!uv_pipe_pending_count(pipe)) {
        rp_free(buf->base);
        return;
    }

    fprintf(stderr, "reactor actor ipc recv: %d %p\n", status, reactor_ext->reactor);
    rp_reactor_data_receive(pipe, status, buf);
}

static void rp_reactor_data_receive(uv_pipe_t *pipe, int status, const uv_buf_t *buf)
{
    rp_reactor_ext_t *reactor_ext = (rp_reactor_ext_t *) buf->base;
    rp_reactor_data_send_req_t *req;
    rp_stream_t *result_stream;

    if(status <= 0){
        rp_free(buf->base);
        return;
    }
    req = (rp_reactor_data_send_req_t *) reactor_ext->data;

    switch(req->type) {
        case RP_SEND:
            fprintf(stderr, "rp_send:%p %p\n", req, buf->base);
            if((result_stream = rp_accept_client(pipe, reactor_ext->reactor)) != NULL) {
                reactor_ext->reactor->cb.dgram.send(reactor_ext->reactor, &req->payload.send, result_stream);
            }
            break;
        case RP_RECV:
            reactor_ext->reactor->cb.dgram.data_recv(reactor_ext->reactor->server, &req->payload.recv);
            rp_free(buf->base);
            break;
        default:
            rp_free(buf->base);
            break;
    }
}

static void reactor_free(rp_reactor_t *reactor)
{
    rp_free(reactor);
}

static void reactors_destroy(zval *reactor_p)
{
    rp_reactor_t *reactor = Z_PTR_P(reactor_p);

    if(reactor->server){
        zend_object_ptr_dtor(reactor->server);
    }

    if(reactor->reactor_free_cb){
        reactor->reactor_free_cb(reactor);
    }
}

static void reactor_async_init_free(zval *data)
{
    rp_free(Z_PTR_P(data));
}

static void write2_cb(reactor_ipc_send_req_t *req, int status)
{
    uv_close((uv_handle_t *) req->client, (uv_close_cb) req->close_cb);
    rp_free(req);
}

static void rp_init_actor_server(int worker_ipc_fd, int worker_data_fd)
{
    zval *current;
    rp_reactor_t *reactor;
    uv_pipe_init(&main_loop, &ipc_pipe, 1);
    uv_pipe_init(&main_loop, &data_pipe, 0);
    uv_pipe_open(&ipc_pipe, worker_ipc_fd);
    uv_pipe_open(&data_pipe, worker_data_fd);
    uv_read_start((uv_stream_t*) &ipc_pipe, rp_alloc_buffer, (uv_read_cb) rp_reactor_actor_ipc_receive);
    uv_read_start((uv_stream_t*) &data_pipe, rp_alloc_buffer, (uv_read_cb) rp_reactor_data_receive);

    for(
        zend_hash_internal_pointer_reset(&rp_reactors);
        current = zend_hash_get_current_data(&rp_reactors);
        zend_hash_move_forward(&rp_reactors)
    ) {
        reactor = Z_PTR_P(current);
        if(reactor->server_init_cb) {
            reactor->server_init_cb(reactor);
        }
    }
}

static int rp_init_worker_server(int worker_ipc_fd, int worker_data_fd)
{
    int ret;
    zval *current;
    rp_reactor_t *reactor;

    if((ret = uv_pipe_init(&main_loop, &ipc_pipe, 1)) < 0){
        return ret;
    }

    if((ret = uv_pipe_open(&ipc_pipe, worker_ipc_fd)) < 0){
        return ret;
    }

    if((ret = uv_read_start((uv_stream_t*) &ipc_pipe, rp_alloc_buffer, (uv_read_cb) rp_reactor_ipc_receive)) < 0){
        return ret;
    }

    if((ret = uv_pipe_init(&main_loop, &data_pipe, 1)) < 0){
        return ret;
    }

    if((ret = uv_pipe_open(&data_pipe, worker_data_fd)) < 0){
        return ret;
    }

    if((ret = uv_read_start((uv_stream_t*) &data_pipe, rp_alloc_buffer, (uv_read_cb) rp_reactor_data_receive)) < 0){
        return ret;
    }

    for(
        zend_hash_internal_pointer_reset(&rp_reactors);
        current = zend_hash_get_current_data(&rp_reactors);
        zend_hash_move_forward(&rp_reactors)
    ) {
        reactor = Z_PTR_P(current);
        if(reactor->worker_init_cb) {
            reactor->worker_init_cb(reactor);
        }
    }

    fprintf(stderr, "data pipe: %d %d\n", getpid(), ret);
    return ret;
}

int rp_reactors_count()
{
    return zend_hash_num_elements(&rp_reactors);
}

int rp_init_reactor(int worker_ipc_fd, int worker_data_fd, int routine_ipc_fd)
{
    int ret = 0;
    uv_loop_init(&main_loop);
    main_loop.data = &main_loop;
    switch(rp_get_task_type()) {
        case ACTOR:
            if(rp_reactors_count() > 0) {
                rp_register_pdeath_sig(&main_loop, SIGINT, rp_signal_hup_handler);
                rp_init_actor_server(worker_ipc_fd, worker_data_fd);
            }
            rp_reactor_async_init_execute();
            break;
        case WORKER:
            rp_register_pdeath_sig(&main_loop, SIGHUP, rp_signal_hup_handler);
            rp_init_worker_server(worker_ipc_fd, worker_data_fd);
            uv_pipe_init(&main_loop, &routine_pipe, 1);
            uv_pipe_open(&routine_pipe, routine_ipc_fd);
            fprintf(stderr, "worker: %d\n", getpid());
            break;            
        case ROUTINE:
            rp_register_pdeath_sig(&main_loop, SIGHUP, rp_signal_hup_handler);
            rp_init_routine_server(routine_ipc_fd);
            break;
        case WORKER_MANAGER:
            break;
        case ROUTINE_MANAGER:
            break;
    }
    return ret;
}

void rp_reactors_init()
{
    actor_pid = getpid();
    zend_hash_init(&rp_reactors, 10, NULL, reactors_destroy, 0);
    zend_hash_init(&rp_reactors_async_inits, 10, NULL, reactor_async_init_free, 0);
}

void rp_reactors_destroy()
{
    zend_hash_destroy(&rp_reactors);
    zend_hash_destroy(&rp_reactors_async_inits);
}

rp_reactor_t *rp_reactor_init(rp_reactor_t *reactor)
{
    memset(reactor, 0, sizeof(rp_reactor_t));
}

rp_reactor_t *rp_reactors_add_new(zval *server)
{
    rp_reactor_t *reactor = rp_malloc(sizeof(rp_reactor_t));
    rp_reactor_init(reactor);
    reactor->reactor_free_cb = reactor_free;
    zend_hash_next_index_insert_ptr(&rp_reactors, reactor);
    Z_ADDREF_P(server);
    reactor->server = Z_OBJ_P(server);
    return reactor;
}

int rp_reactor_data_send(rp_reactor_t *reactor, uv_close_cb close_cb, char *data, size_t data_len)
{
    reactor_data_send_req_t *send_req = rp_malloc(sizeof(reactor_data_send_req_t) + data_len);
    send_req->close_cb = close_cb;
    send_req->reactor_ext.reactor = reactor;
    send_req->reactor_ext.data_len = data_len;
    memcpy(&send_req->reactor_ext.data, data, data_len);
    send_req->buf.base = (char *) &send_req->reactor_ext;
    send_req->buf.len = sizeof(rp_reactor_ext_t) + data_len;
    fprintf(stderr, "data send: %.*s\n", data_len, data);
    return uv_write((uv_write_t *) send_req, (uv_stream_t *) &data_pipe, &send_req->buf, 1, (uv_write_cb) rp_free_cb);
}

int rp_reactor_ipc_send_ex(rp_reactor_t *reactor, uv_stream_t *client, uv_close_cb close_cb, char *data, size_t data_len, uv_stream_t *ipc)
{
    reactor_ipc_send_req_t *send_req = rp_malloc(sizeof(reactor_ipc_send_req_t) + data_len);
    send_req->close_cb = close_cb;
    send_req->client = client;
    send_req->reactor_ext.reactor = reactor;
    send_req->reactor_ext.data_len = data_len;
    memcpy(&send_req->reactor_ext.data, data, data_len);
    send_req->buf.base = (char *) &send_req->reactor_ext;
    send_req->buf.len = sizeof(rp_reactor_ext_t) + data_len;
    return uv_write2((uv_write_t *) send_req, ipc, &send_req->buf, 1, client, (uv_write_cb) write2_cb);
}

static void rp_signal_hup_handler(uv_signal_t* signal, int signum)
{
    uv_signal_stop(signal);
    uv_stop(&main_loop);
}

void rp_reactor_async_init(rp_reactor_async_init_cb callback, void *data)
{
    async_init_t *async_init;

    if(main_loop_inited()){
        callback(data);
        return;
    }

    async_init = rp_malloc(sizeof(async_init_t));
    async_init->callback = callback;
    async_init->data = data;
    zend_hash_next_index_insert_ptr(&rp_reactors_async_inits, async_init);
}

void rp_reactor_async_init_execute()
{
    zval *current;
    async_init_t *async_init;
    for(
            zend_hash_internal_pointer_reset(&rp_reactors_async_inits);
            current = zend_hash_get_current_data(&rp_reactors_async_inits);
            zend_hash_move_forward(&rp_reactors_async_inits)
            ) {
        async_init = Z_PTR_P(current);
        async_init->callback(async_init->data);
    }
}
