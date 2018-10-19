#include "respondphp.h"
#include "interface/stream_server_interface.h"

DECLARE_FUNCTION_ENTRY(respond_stream_server_interface) =
{
    PHP_ABSTRACT_ME(respond_stream_server_interface, close, ARGINFO(respond_stream_server_interface, close))
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_stream_server_interface)
{
    INIT_CLASS(respond_stream_server_interface, "Respond\\Stream\\ServerInterface");
    REGISTER_INTERNAL_INTERFACE(respond_stream_server_interface);
    zend_class_implements(
        CLASS_ENTRY(respond_stream_server_interface),
        1,
        CLASS_ENTRY(respond_event_event_emitter_interface)
    );

}
