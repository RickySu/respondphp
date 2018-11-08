#include "respondphp.h"
#include "interface/socket_connector_interface.h"

DECLARE_FUNCTION_ENTRY(respond_socket_connector_interface) =
{
    ZEND_FENTRY(__construct, NULL, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_ABSTRACT)
    ZEND_FENTRY(connect, NULL, ARGINFO(respond_socket_connector_interface, connect), ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT | ZEND_ACC_STATIC)
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_socket_connector_interface)
{
    INIT_CLASS(respond_socket_connector_interface, "Respond\\Socket\\ConnectorInterface");
    REGISTER_INTERNAL_INTERFACE(respond_socket_connector_interface);
}
