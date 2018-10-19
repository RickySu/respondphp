#ifndef _RP_STREAM_SERVER_INTERFACE_H
#define _RP_STREAM_SERVER_INTERFACE_H

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_server_interface_close(ce) \
    ZEND_BEGIN_ARG_INFO(ARGINFO(ce, close), 0) \
    RP_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_server_interface(ce) \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_server_interface_close(ce)

TRAIT_FUNCTION_ARG_INFO(respond_stream_server_interface, respond_stream_server_interface);
#endif
