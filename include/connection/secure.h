#ifndef RP_CONNECTION_SECURE_H
#define RP_CONNECTION_SECURE_H
#include "internal/event_emitter.h"
#include "internal/socket_connection.h"
#include "interface/stream_readable_stream_interface.h"
#include "interface/stream_writable_stream_interface.h"
#include "connection/connection.h"

CLASS_ENTRY_FUNCTION_D(respond_connection_secure);

typedef struct {
    uint flag;
    SSL *ssl;
    BIO *read_bio;
    BIO *write_bio;
    rp_event_emitter_internal_cb handshake;
    void *creater_resource;
    rp_connection_connection_ext_t *connection;
    event_hook_t event_hook;
    zend_object  zo;
} rp_connection_secure_ext_t;

PHP_METHOD(respond_connection_secure, __construct);
PHP_METHOD(respond_connection_secure, isReadable);
PHP_METHOD(respond_connection_secure, isWritable);
PHP_METHOD(respond_connection_secure, close);
PHP_METHOD(respond_connection_secure, write);
PHP_METHOD(respond_connection_secure, end);

TRAIT_FUNCTION_ARG_INFO(respond_connection_secure, stream_readable_stream);
TRAIT_FUNCTION_ARG_INFO(respond_connection_secure, stream_writable_stream);

TRAIT_PHP_METHOD(respond_connection_secure, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_connection_secure, event_emitter);

TRAIT_PHP_METHOD(respond_connection_secure, socket_connection);
TRAIT_FUNCTION_ARG_INFO(respond_connection_secure, socket_connection);


static zend_always_inline int write_bio_to_socket(rp_connection_secure_ext_t *resource)
{
    char buffer[256];
    int n_read, ret;

    while(1){
        if((n_read = BIO_read(resource->write_bio, buffer, sizeof(buffer))) <= 0){
            ret = 0;
            break;
        }
        if(!(ret = rp_connection_write(resource->connection, buffer, n_read))){
            break;
        }
    }

    return ret;
}

#endif //RP_CONNECTION_SECURE_H