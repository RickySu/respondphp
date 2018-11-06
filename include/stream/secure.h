#ifndef RP_STREAM_SECURE_H
#define RP_STREAM_SECURE_H
#include "internal/event_emitter.h"
#include "internal/stream_connection.h"
#include "interface/stream_readable_stream_interface.h"
#include "interface/stream_writable_stream_interface.h"
#include "stream/connection.h"

CLASS_ENTRY_FUNCTION_D(respond_stream_secure);

typedef struct {
    rp_connection_methods_t connection_methods;
    uint flag;
    SSL_CTX *ctx;
    SSL *ssl;
    BIO *read_bio;
    BIO *write_bio;
    rp_event_emitter_internal_cb handshake;
    void *creater_resource;
    rp_stream_connection_ext_t *connection;
    event_hook_t event_hook;
    zend_object  zo;
} rp_stream_secure_ext_t;

PHP_METHOD(respond_stream_secure, __construct);
PHP_METHOD(respond_stream_secure, isReadable);
PHP_METHOD(respond_stream_secure, isWritable);
PHP_METHOD(respond_stream_secure, close);
PHP_METHOD(respond_stream_secure, write);
PHP_METHOD(respond_stream_secure, end);

TRAIT_FUNCTION_ARG_INFO(respond_stream_secure, stream_readable_stream);
TRAIT_FUNCTION_ARG_INFO(respond_stream_secure, stream_writable_stream);

TRAIT_PHP_METHOD(respond_stream_secure, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_stream_secure, event_emitter);

TRAIT_PHP_METHOD(respond_stream_secure, stream_connection);
TRAIT_FUNCTION_ARG_INFO(respond_stream_secure, stream_connection);


static zend_always_inline zend_bool write_bio_to_socket(rp_stream_secure_ext_t *resource)
{
    char buffer[256];
    int n_read;
    zend_bool ret;

    while(1){
        if((n_read = BIO_read(resource->write_bio, buffer, sizeof(buffer))) <= 0){
            ret = 1;
            break;
        }
        if(!(ret = resource->connection->connection_methods.write(resource->connection, buffer, n_read))){
            break;
        }
    }

    return ret;
}

#endif //RP_STREAM_SECURE_H
