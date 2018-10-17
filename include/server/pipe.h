#ifndef _RP_SERVER_PIPE_H
#define _RP_SERVER_PIPE_H
#include "internal/event_emitter.h"

CLASS_ENTRY_FUNCTION_D(respond_server_pipe);

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_pipe, __construct), 0)
    ZEND_ARG_INFO(0, socket_path)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    event_hook_t event_hook;
    rp_reactor_t *reactor;
    zend_object zo;
} rp_pipe_ext_t;

PHP_METHOD(respond_server_pipe, close);
PHP_METHOD(respond_server_pipe, __construct);

TRAIT_PHP_METHOD(respond_server_pipe, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_pipe, event_emitter);
#endif
