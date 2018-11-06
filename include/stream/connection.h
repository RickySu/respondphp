#ifndef RP_STREAM_CONNECTION_H
#define RP_STREAM_CONNECTION_H
#include "internal/event_emitter.h"
#include "internal/stream_connection.h"
#include "interface/stream_readable_stream_interface.h"
#include "interface/stream_writable_stream_interface.h"

CLASS_ENTRY_FUNCTION_D(respond_stream_connection);

typedef struct {
    rp_connection_methods_t connection_methods;
    uint flag;
    rp_stream_t    *stream;
    rp_write_req_t *close_write_req;
    event_hook_t event_hook;
    zend_object  zo;
} rp_stream_connection_ext_t;

PHP_METHOD(respond_stream_connection, __construct);
PHP_METHOD(respond_stream_connection, isReadable);
PHP_METHOD(respond_stream_connection, isWritable);
PHP_METHOD(respond_stream_connection, close);
PHP_METHOD(respond_stream_connection, write);
PHP_METHOD(respond_stream_connection, end);

TRAIT_FUNCTION_ARG_INFO(respond_stream_connection, stream_readable_stream);
TRAIT_FUNCTION_ARG_INFO(respond_stream_connection, stream_writable_stream);

TRAIT_PHP_METHOD(respond_stream_connection, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_stream_connection, event_emitter);

TRAIT_PHP_METHOD(respond_stream_connection, stream_connection);
TRAIT_FUNCTION_ARG_INFO(respond_stream_connection, stream_connection);
#endif //RP_STREAM_CONNECTION_H
