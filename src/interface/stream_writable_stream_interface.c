#include "respondphp.h"
#include "interface/stream_writable_stream_interface.h"
DECLARE_FUNCTION_ENTRY(respond_stream_writable_stream_interface) =
{
    PHP_ABSTRACT_ME(respond_stream_writable_stream_interface, isWritable, ARGINFO(respond_stream_writable_stream_interface, isWritable))
    PHP_ABSTRACT_ME(respond_stream_writable_stream_interface, write, ARGINFO(respond_stream_writable_stream_interface, write))
    PHP_ABSTRACT_ME(respond_stream_writable_stream_interface, end, ARGINFO(respond_stream_writable_stream_interface, end))
    PHP_ABSTRACT_ME(respond_stream_writable_stream_interface, close, ARGINFO(respond_stream_writable_stream_interface, close))
    PHP_FE_END
};
CLASS_ENTRY_FUNCTION_D(respond_stream_writable_stream_interface)
{
    INIT_CLASS(respond_stream_writable_stream_interface, "Respond\\Stream\\WritableStreamInterface");
    REGISTER_INTERNAL_INTERFACE(respond_stream_writable_stream_interface);
}
