#include "respondphp.h"
#include "interface/stream_readable_stream_interface.h"
DECLARE_FUNCTION_ENTRY(respond_stream_readable_stream_interface) =
{
    PHP_ABSTRACT_ME(respond_stream_readable_stream_interface, isReadable, ARGINFO(respond_stream_readable_stream_interface, isReadable))
    PHP_ABSTRACT_ME(respond_stream_readable_stream_interface, close, ARGINFO(respond_stream_readable_stream_interface, close))
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_stream_readable_stream_interface)
{
    INIT_CLASS(respond_stream_readable_stream_interface, "Respond\\Stream\\ReadableStreamInterface");
    REGISTER_INTERNAL_INTERFACE(respond_stream_readable_stream_interface);
}
