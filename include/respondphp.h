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
extern uv_pipe_t routine_pipe;

PHP_MINIT_FUNCTION(respondphp);
PHP_MSHUTDOWN_FUNCTION(respondphp);
PHP_MINFO_FUNCTION(respondphp);
PHP_RINIT_FUNCTION(respondphp);
PHP_RSHUTDOWN_FUNCTION(respondphp);

extern zend_module_entry respondphp_module_entry;

DECLARE_CLASS_ENTRY(respond_event_loop);
DECLARE_CLASS_ENTRY(respond_server_tcp);
DECLARE_CLASS_ENTRY(respond_server_routine);
DECLARE_CLASS_ENTRY(respond_connection_connection);
DECLARE_CLASS_ENTRY(respond_event_event_emitter_interface);
DECLARE_CLASS_ENTRY(respond_stream_writable_stream_interface);
DECLARE_CLASS_ENTRY(respond_stream_readable_stream_interface);
DECLARE_CLASS_ENTRY(respond_socket_connection_interface);

int rp_init_routine_manager();
int rp_init_worker_manager();
int rp_init_reactor(int worker_fd, int routine_fd);
rp_task_type_t rp_get_task_type();
void rp_set_task_type(rp_task_type_t type);
rp_reactor_t *rp_reactor_add();
void rp_reactor_send_ex(rp_reactor_t *reactor, uv_stream_t *client, uv_close_cb *close_cb, char *data, size_t data_len, uv_stream_t *ipc);
#define rp_reactor_send(reactor, client, close_cb) \
do{\
    rp_reactor_send_ex(reactor, client, close_cb, NULL, 0, (uv_stream_t *) &ipc_pipe);\
} while(0)

void rp_connection_factory(rp_client_t *client, zval *connection);
#endif
