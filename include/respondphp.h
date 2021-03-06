#ifndef _RP_RESPONDPHP_H
#define _RP_RESPONDPHP_H

//#ifdef HAVE_CONFIG_H
    #include "config.h"
//#endif

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

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include "ssl_detect.h"
#endif

#include <sys/prctl.h>
#include <php.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_exceptions.h>
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
extern zend_class_entry *rp_promise_ce;
extern zend_module_entry respondphp_module_entry;
extern pid_t actor_pid;

#define main_loop_inited() (main_loop.data == &main_loop)

PHP_MINIT_FUNCTION(respondphp);
PHP_MSHUTDOWN_FUNCTION(respondphp);
PHP_MINFO_FUNCTION(respondphp);
PHP_RINIT_FUNCTION(respondphp);
PHP_RSHUTDOWN_FUNCTION(respondphp);

DECLARE_CLASS_ENTRY(respond_event_loop);
DECLARE_CLASS_ENTRY(respond_connector_pipe);
DECLARE_CLASS_ENTRY(respond_connector_tcp);
DECLARE_CLASS_ENTRY(respond_server_http);
DECLARE_CLASS_ENTRY(respond_server_tcp);
DECLARE_CLASS_ENTRY(respond_server_udp);
DECLARE_CLASS_ENTRY(respond_server_pipe);
DECLARE_CLASS_ENTRY(respond_server_routine);
DECLARE_CLASS_ENTRY(respond_stream_connection);
DECLARE_CLASS_ENTRY(respond_event_event_emitter_interface);
DECLARE_CLASS_ENTRY(respond_stream_server_interface);
DECLARE_CLASS_ENTRY(respond_stream_writable_stream_interface);
DECLARE_CLASS_ENTRY(respond_stream_readable_stream_interface);
DECLARE_CLASS_ENTRY(respond_stream_connection_interface);
DECLARE_CLASS_ENTRY(respond_socket_connector_interface);
DECLARE_CLASS_ENTRY(respond_async_promise_interface);
DECLARE_CLASS_ENTRY(respond_async_cancelable_promise);
DECLARE_CLASS_ENTRY(respond_network_resolver);
DECLARE_CLASS_ENTRY(respond_system_timer);

#ifdef HAVE_OPENSSL
DECLARE_CLASS_ENTRY(respond_server_secure);
DECLARE_CLASS_ENTRY(respond_stream_secure);
DECLARE_CLASS_ENTRY(respond_connector_secure);
#endif

void rp_init_routine_manager(int *routine_fd);
void rp_init_worker_manager(int *worker_fd, int *worker_data_fd);
int rp_init_reactor(int worker_ipc_fd, int work_data_fd, int routine_ipc_fd);
rp_task_type_t rp_get_task_type();
void rp_set_task_type(rp_task_type_t type);
rp_reactor_t *rp_reactors_add_new(zval *server);
rp_reactor_t *rp_reactor_init(rp_reactor_t *reactor);
void rp_reactors_init();
void rp_reactors_destroy();
int rp_reactors_count();
int rp_reactor_data_send(rp_reactor_t *reactor, uv_close_cb close_cb, char *data, size_t data_len);
int rp_reactor_ipc_send_ex(rp_reactor_t *reactor, uv_stream_t *client, uv_close_cb close_cb, char *data, size_t data_len, uv_stream_t *ipc);
#define rp_reactor_ipc_send(reactor, client, close_cb) rp_reactor_ipc_send_ex(reactor, client, close_cb, NULL, 0, (uv_stream_t *) &ipc_pipe)
void rp_stream_connection_factory(rp_stream_t *client, zval *connection);
void rp_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void rp_alloc_buffer_zend_string(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void rp_free_cb(void *data);
void rp_reactor_async_init_ex(rp_reactor_async_cb init_callback, rp_reactor_async_cb free_callback, void *data);
void rp_reactor_async_init_execute();
void rp_reactor_async_free_execute();

#define rp_reactor_async_init(init_callback, data) rp_reactor_async_init_ex(init_callback, NULL, data)

#ifdef HAVE_OPENSSL
void rp_stream_secure_factory(SSL *ssl, zval *connection_connection, zval *connection_secure);
#endif

static zend_always_inline zend_string *rp_init_empty_zend_string(size_t n)
{
    zend_string *string;
    string = zend_string_alloc(n, 0);
    string->len = 0;
    return string;
}

static zend_always_inline void rp_reject_promise(zval *promise, zval *result)
{
    zend_call_method_with_1_params(promise, Z_OBJCE_P(promise), NULL, "reject", NULL, result);
}

static zend_always_inline void rp_resolve_promise(zval *promise, zval *result)
{
    zend_call_method_with_1_params(promise, Z_OBJCE_P(promise), NULL, "resolve", NULL, result);
}

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

static zend_always_inline zend_class_entry *rp_fetch_ce(char *class_name, size_t class_name_len)
{
    zend_string *z_class_name;
    zend_class_entry *ce;
    z_class_name = zend_string_init(class_name, class_name_len, 0);
    ce = zend_fetch_class(z_class_name, ZEND_FETCH_CLASS_AUTO);
    zend_string_release(z_class_name);
    return ce;
}

static zend_always_inline void rp_make_promise_object_ex(zval *promise, zend_class_entry *ce)
{
    object_init_ex(promise, ce);
}

static zend_always_inline void rp_make_promise_object(zval *promise)
{
    rp_make_promise_object_ex(promise, rp_promise_ce);
}

static zend_always_inline zend_bool rp_addr(rp_reactor_addr_t *addr, zend_string *host, int16_t port)
{
    if(host == NULL) {
        host = zend_string_init("::", 2, 1);
    }

    if (memchr(host->val, ':', host->len) == NULL) {
        if (uv_ip4_addr(host->val, port, &addr->sockaddr) != 0) {
            return 0;
        }
    }
    else {
        if (uv_ip6_addr(host->val, port, &addr->sockaddr6) != 0) {
            return 0;
        }
    }
    return 1;
}

static zend_always_inline void rp_reject_promise_long(zval *promise, int err)
{
    zval exception, reason;
    ZVAL_LONG(&reason, err);
    object_init_ex(&exception, zend_ce_exception);
    zend_call_method_with_1_params(&exception, NULL, &Z_OBJCE(exception)->constructor, "__construct", NULL, &reason);
    rp_reject_promise(promise, &exception);
    ZVAL_PTR_DTOR(&exception);
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
