#ifndef _RP_SERVER_UDP_H
#define _RP_SERVER_UDP_H
#include "internal/event_emitter.h"

CLASS_ENTRY_FUNCTION_D(respond_server_udp);

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_udp, __construct), 0)
    ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
ZEND_END_ARG_INFO()

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_server_udp, send), "Respond\\Async\\PromiseInterface", 0)
    ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    event_hook_t event_hook;
    rp_reactor_t *reactor;
    zend_object zo;
} rp_udp_ext_t;

typedef struct {
    uv_udp_send_t send;
    uv_buf_t buf;
    rp_reactor_data_send_req_payload_send_t *payload;
    rp_stream_t *result_stream;
} rp_udp_send_t;

typedef struct {
    uv_udp_send_t send;
    zend_string *data;
    uv_pipe_t worker_pipe;
    uv_pipe_t actor_pipe;
    zval promise;
} rp_udp_send_resul_t;

PHP_METHOD(respond_server_udp, close);
PHP_METHOD(respond_server_udp, send);
PHP_METHOD(respond_server_udp, __construct);

TRAIT_PHP_METHOD(respond_server_udp, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_udp, event_emitter);
#endif
