#include "respondphp.h"
#include "interface/socket_connection_interface.h"

CLASS_ENTRY_FUNCTION_D(respond_socket_connection_interface)
{
    INIT_CLASS(respond_socket_connection_interface, "Respond\\Socket\\ConnectionInterface");
    REGISTER_INTERNAL_INTERFACE(respond_socket_connection_interface);
}
