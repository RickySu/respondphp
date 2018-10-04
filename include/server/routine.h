#ifndef _RP_SERVER_ROUTINE_H
#define _RP_SERVER_ROUTINE_H

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_routine, __construct), 0)
    ZEND_ARG_CALLABLE_INFO(0, execution, 0)
ZEND_END_ARG_INFO()

RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_server_routine, execute), PREDEFINED_PHP_Respond_Async_PromiseInterface, 0)
    ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    rp_reactor_t *reactor;
    zval execution;
    fcall_info_t execution_fci;
    HashTable routine_executions;
    zend_object zo;
} rp_routine_ext_t;

typedef struct write_req_s{
    uv_write_t uv_write;
    uv_buf_t buf;
} write_req_t;

typedef struct {
    uv_pipe_t pipe;
    int fd;
    zval promise;
    rp_routine_ext_t *resource;
} routine_execution_t;

PHP_METHOD(respond_server_routine, __construct);
PHP_METHOD(respond_server_routine, execute);
#endif
