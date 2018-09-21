#ifndef _RP_SOCKET_CONNECTION_INTERFACE_H
#define _RP_SOCKET_CONNECTION_INTERFACE_H

#define TRAIT_FUNCTION_ARG_INFO_respond_socket_connection_interface_getRemoteAddress(ce) \
    RP_BEGIN_ARG_INFO(ARGINFO(ce, getRemoteAddress), IS_STRING, 1) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_socket_connection_interface_getLocalAddress(ce) \
    RP_BEGIN_ARG_INFO(ARGINFO(ce, getLocalAddress), IS_STRING, 1) \
    RP_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_socket_connection_interface(ce) \
    TRAIT_FUNCTION_ARG_INFO_respond_socket_connection_interface_getRemoteAddress(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_socket_connection_interface_getLocalAddress(ce)

TRAIT_FUNCTION_ARG_INFO(respond_socket_connection_interface, respond_socket_connection_interface);
DECLARE_FUNCTION_ENTRY(respond_socket_connection_interface) =
{
    PHP_ABSTRACT_ME(respond_socket_connection_interface, getRemoteAddress, ARGINFO(respond_socket_connection_interface, getRemoteAddress))
    PHP_ABSTRACT_ME(respond_socket_connection_interface, getLocalAddress, ARGINFO(respond_socket_connection_interface, getLocalAddress))
    PHP_FE_END
};
#endif
