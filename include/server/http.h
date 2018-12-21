#ifndef _RP_SERVER_HTTP_H
#define _RP_SERVER_HTTP_H
#include "internal/event_emitter.h"
#include "stream/connection.h"
#include "picohttpparser.h"
#include "ext/standard/php_string.h"

#define RP_HTTP_HEADER_MAX 2048
#define RP_HTTP_PARSE_HEADER 0
#define RP_HTTP_PARSE_BODY 1

CLASS_ENTRY_FUNCTION_D(respond_server_http);

ZEND_BEGIN_ARG_INFO(ARGINFO(respond_server_http, __construct), 0)
    ZEND_ARG_OBJ_INFO(0, socket, Respond\\Stream\\ServerInterface, 0)
    ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

typedef struct {
    uint flag;
    zend_object *socket_zo;
    event_hook_t event_hook;
    zend_object zo;
} rp_server_http_ext_t;

typedef struct {
    rp_stream_connection_ext_t *connection_resource;
    rp_server_http_ext_t *server_resource;
    zend_string *buffer;
    uint flag;
} rp_http_connection_t;

PHP_METHOD(respond_server_http, close);
PHP_METHOD(respond_server_http, __construct);

TRAIT_PHP_METHOD(respond_server_http, event_emitter);
TRAIT_FUNCTION_ARG_INFO(respond_server_http, event_emitter);
#endif
