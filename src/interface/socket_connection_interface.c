#include "respondphp.h"
#include "interface/socket_connection_interface.h"

DECLARE_FUNCTION_ENTRY(respond_socket_connection_interface) =
{
    PHP_ABSTRACT_ME(respond_socket_connection_interface, getRemoteAddress, ARGINFO(respond_socket_connection_interface, getRemoteAddress))
    PHP_ABSTRACT_ME(respond_socket_connection_interface, getLocalAddress, ARGINFO(respond_socket_connection_interface, getLocalAddress))
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_socket_connection_interface)
{
    INIT_CLASS(respond_socket_connection_interface, "Respond\\Socket\\ConnectionInterface");
    REGISTER_INTERNAL_INTERFACE(respond_socket_connection_interface);
    zend_class_implements(
        CLASS_ENTRY(respond_socket_connection_interface),
        3,
        CLASS_ENTRY(respond_stream_readable_stream_interface),
        CLASS_ENTRY(respond_stream_writable_stream_interface),
        CLASS_ENTRY(respond_event_event_emitter_interface)
    );

}
