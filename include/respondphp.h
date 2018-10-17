#ifndef _RP_RESPONDPHP_H
#define _RP_RESPONDPHP_H

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef HAVE_DEBUG
    #define RP_ASSERT(exp) ZEND_ASSERT(exp)
#else
    #define RP_ASSERT(exp)
#endif

#ifdef HAVE_JEMALLOC
    #include <jemalloc/jemalloc.h>
    #define rp_malloc malloc
    #define rp_calloc calloc
    #define rp_realloc realloc
    #define rp_free free
#else
    #define rp_malloc emalloc
    #define rp_calloc ecalloc
    #define rp_realloc erealloc
    #define rp_free efree
#endif

#include <sys/prctl.h>
#include <php.h>
#include <ext/standard/info.h>
#include <uv.h>
#include "config.h"
#include "predefine.h"
#include "types.h"
#include "common.h"
#include "fcall.h"

extern uv_loop_t main_loop;
extern uv_pipe_t ipc_pipe;
extern uv_pipe_t data_pipe;
extern uv_pipe_t routine_pipe;
extern zend_module_entry respondphp_module_entry;

PHP_MINIT_FUNCTION(respondphp);
PHP_MSHUTDOWN_FUNCTION(respondphp);
PHP_MINFO_FUNCTION(respondphp);
PHP_RINIT_FUNCTION(respondphp);
PHP_RSHUTDOWN_FUNCTION(respondphp);

DECLARE_CLASS_ENTRY(respond_event_loop);
DECLARE_CLASS_ENTRY(respond_connector_tcp);
DECLARE_CLASS_ENTRY(respond_server_tcp);
DECLARE_CLASS_ENTRY(respond_server_udp);
DECLARE_CLASS_ENTRY(respond_server_pipe);
DECLARE_CLASS_ENTRY(respond_server_routine);
DECLARE_CLASS_ENTRY(respond_connection_connection);
DECLARE_CLASS_ENTRY(respond_event_event_emitter_interface);
DECLARE_CLASS_ENTRY(respond_stream_writable_stream_interface);
DECLARE_CLASS_ENTRY(respond_stream_readable_stream_interface);
DECLARE_CLASS_ENTRY(respond_socket_connection_interface);
DECLARE_CLASS_ENTRY(respond_socket_connector_interface);

void rp_init_routine_manager(int *routine_fd);
void rp_init_worker_manager(int *worker_fd, int *worker_data_fd);
int rp_init_reactor(int worker_ipc_fd, int work_data_fd, int routine_ipc_fd);
rp_task_type_t rp_get_task_type();
void rp_set_task_type(rp_task_type_t type);
rp_reactor_t *rp_reactor_add();
rp_reactor_t *rp_reactor_get_head();
void rp_reactor_destroy();
int rp_reactor_data_send(rp_reactor_t *reactor, uv_close_cb close_cb, char *data, size_t data_len);
int rp_reactor_ipc_send_ex(rp_reactor_t *reactor, uv_stream_t *client, uv_close_cb close_cb, char *data, size_t data_len, uv_stream_t *ipc);
#define rp_reactor_ipc_send(reactor, client, close_cb) rp_reactor_ipc_send_ex(reactor, client, close_cb, NULL, 0, (uv_stream_t *) &ipc_pipe)
void rp_connection_factory(rp_stream_t *client, zval *connection);
void rp_make_promise_object(zval *promise);
void rp_reject_promise(zval *promise, zval *result);
void rp_resolve_promise(zval *promise, zval *result);
void rp_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void rp_alloc_buffer_zend_string(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void rp_close_cb_release(uv_handle_t* handle);

static zend_always_inline rp_write_req_t *rp_make_write_req(char *data, size_t data_len)
{
    rp_write_req_t *req;
    req = rp_malloc(sizeof(rp_write_req_t) + data_len - 1);
    req->buf.base = req->data;
    req->buf.len = data_len;
    memcpy(req->buf.base, data, data_len);
    return req;
}

static zend_always_inline void sock_addr(struct sockaddr *sa, char *ip_name, size_t ip_len, u_int16_t *port)
{

    if(sa->sa_family == AF_INET) {
        inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), ip_name, ip_len),
        *port = ntohs(((struct sockaddr_in *)sa)->sin_port);
        return;
    }

    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), ip_name, ip_len),
    *port = ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
}

static zend_always_inline void zend_object_ptr_dtor(zend_object *zo)
{
    zval gc;
    ZVAL_OBJ(&gc, zo);
    ZVAL_PTR_DTOR(&gc);
}

#ifdef HAVE_PR_SET_PDEATHSIG
#define DETTACH_SESSION setsid
extern  uv_signal_t signal_handle;
static zend_always_inline void rp_register_pdeath_sig(uv_loop_t *loop, int signum, uv_signal_cb signal_cb)
{
    uv_signal_init(loop, &signal_handle);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    uv_signal_start(&signal_handle, signal_cb, signum);
}
#else
#define DETTACH_SESSION()
#define rp_register_pdeath_sig(x, y, z)
#endif

#endif
