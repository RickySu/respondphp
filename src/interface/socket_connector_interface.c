#include "respondphp.h"
#include "interface/socket_connector_interface.h"

DECLARE_FUNCTION_ENTRY(respond_socket_connector_interface) =
{
    PHP_ABSTRACT_ME(respond_socket_connector_interface, connect, ARGINFO(respond_socket_connector_interface, connect))
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_socket_connector_interface)
{
    INIT_CLASS(respond_socket_connector_interface, "Respond\\Socket\\ConnectorInterface");
    REGISTER_INTERNAL_INTERFACE(respond_socket_connector_interface);
}
