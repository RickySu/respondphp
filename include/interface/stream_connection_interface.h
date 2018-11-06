#ifndef _RP_STREAM_CONNECTION_INTERFACE_H
#define _RP_STREAM_CONNECTION_INTERFACE_H

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_connection_interface_getRemoteAddress(ce) \
    RP_BEGIN_ARG_INFO(ARGINFO(ce, getRemoteAddress), IS_STRING, 1) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_connection_interface_getLocalAddress(ce) \
    RP_BEGIN_ARG_INFO(ARGINFO(ce, getLocalAddress), IS_STRING, 1) \
    RP_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_connection_interface(ce) \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_connection_interface_getRemoteAddress(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_connection_interface_getLocalAddress(ce)

TRAIT_FUNCTION_ARG_INFO(respond_stream_connection_interface, respond_stream_connection_interface);
#endif
