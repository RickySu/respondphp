#ifndef _RP_SOCKET_CONNECTOR_INTERFACE_H
#define _RP_SOCKET_CONNECTOR_INTERFACE_H

#define TRAIT_FUNCTION_ARG_INFO_respond_socket_connector_interface_connect(ce) \
    RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(ARGINFO(respond_socket_connector_interface, connect), "Respond\\Async\\PromiseInterface", 0) \
    ZEND_ARG_VARIADIC_INFO(0, arguments) \
    RP_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_socket_connector_interface(ce) \
    TRAIT_FUNCTION_ARG_INFO_respond_socket_connector_interface_connect(ce)

TRAIT_FUNCTION_ARG_INFO(respond_socket_connector_interface, respond_socket_connector_interface);
#endif
