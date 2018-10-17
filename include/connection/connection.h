#ifndef RP_CONNECTION_CONNECTION_H
#define RP_CONNECTION_CONNECTION_H
#include "internal/event_emitter.h"
#include "internal/socket_connection.h"
#include "interface/stream_readable_stream_interface.h"
#include "interface/stream_writable_stream_interface.h"

CLASS_ENTRY_FUNCTION_D(respond_connection_connection);

typedef struct {
    uint flag;
    rp_stream_t    *stream;
    rp_write_req_t *close_write_req;
    event_hook_t event_hook;
    zend_object  zo;
} rp_connection_ext_t;

PHP_METHOD(respond_connection_connection, __construct);
PHP_METHOD(respond_connection_connection, isReadable);
PHP_METHOD(respond_connection_connection, isWritable);
PHP_METHOD(respond_connection_connection, close);
PHP_METHOD(respond_connection_connection, write);
PHP_METHOD(respond_connection_connection, end);

TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, stream_readable_stream);
TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, stream_writable_stream);

TRAIT_PHP_METHOD(respond_connection_connection, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, event_emitter);

TRAIT_PHP_METHOD(respond_connection_connection, socket_connection);
TRAIT_FUNCTION_ARG_INFO(respond_connection_connection, socket_connection);
#endif //RP_CONNECTION_CONNECTION_H
