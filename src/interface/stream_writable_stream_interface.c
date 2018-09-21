#include "respondphp.h"
#include "interface/stream_writable_stream_interface.h"

CLASS_ENTRY_FUNCTION_D(respond_stream_writable_stream_interface)
{
    INIT_CLASS(respond_stream_writable_stream_interface, "Respond\\Stream\\WritableStreamInterface");
    REGISTER_INTERNAL_INTERFACE(respond_stream_writable_stream_interface);
}
