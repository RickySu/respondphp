#include "respondphp.h"
#include "interface/stream_readable_stream_interface.h"

CLASS_ENTRY_FUNCTION_D(respond_stream_readable_stream_interface)
{
    INIT_CLASS(respond_stream_readable_stream_interface, "Respond\\Stream\\ReadableStreamInterface");
    REGISTER_INTERNAL_INTERFACE(respond_stream_readable_stream_interface);
}
