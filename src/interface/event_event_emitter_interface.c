#include "respondphp.h"
#include "interface/event_event_emitter_interface.h"

CLASS_ENTRY_FUNCTION_D(respond_event_event_emitter_interface)
{
    INIT_CLASS(respond_event_event_emitter_interface, "Respond\\Event\\EventEmitterInterface");
    REGISTER_INTERNAL_INTERFACE(respond_event_event_emitter_interface);
}
