#ifndef RP_CONNECTION_CONNECTION_H
#define RP_CONNECTION_CONNECTION_H
#include "internal/event_emitter.h"
#include "internal/socket_connection.h"
#include "interface/stream_readable_stream_interface.h"
#include "interface/stream_writable_stream_interface.h"

typedef struct {
    uint flag;
    rp_client_t  *client;
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

DECLARE_FUNCTION_ENTRY(respond_connection_connection) =
{
    PHP_ME(respond_connection_connection, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    PHP_ME(respond_connection_connection, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(respond_connection_connection, isReadable, ARGINFO(respond_stream_readable_stream_interface, isReadable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_connection, isWritable, ARGINFO(respond_stream_writable_stream_interface, isWritable), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_connection, write, ARGINFO(respond_stream_writable_stream_interface, write), ZEND_ACC_PUBLIC) \
    PHP_ME(respond_connection_connection, end, ARGINFO(respond_stream_writable_stream_interface, end), ZEND_ACC_PUBLIC) \
    TRAIT_FUNCTION_ENTRY_ME(respond_connection_connection, event_emitter)
    TRAIT_FUNCTION_ENTRY_ME(respond_connection_connection, socket_connection)
    PHP_FE_END
};

#endif //RP_CONNECTION_CONNECTION_H
