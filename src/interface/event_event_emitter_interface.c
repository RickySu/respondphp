#include "respondphp.h"
#include "interface/event_event_emitter_interface.h"

DECLARE_FUNCTION_ENTRY(respond_event_event_emitter_interface) =
{
    PHP_ABSTRACT_ME(respond_event_event_emitter_interface, on, ARGINFO(respond_event_event_emitter_interface, on))
    PHP_ABSTRACT_ME(respond_event_event_emitter_interface, off, ARGINFO(respond_event_event_emitter_interface, off))
    PHP_ABSTRACT_ME(respond_event_event_emitter_interface, removeListeners, ARGINFO(respond_event_event_emitter_interface, removeListeners))
    PHP_ABSTRACT_ME(respond_event_event_emitter_interface, getListeners, ARGINFO(respond_event_event_emitter_interface, getListeners))
    PHP_ABSTRACT_ME(respond_event_event_emitter_interface, emit, ARGINFO(respond_event_event_emitter_interface, emit))
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_event_event_emitter_interface)
{
    INIT_CLASS(respond_event_event_emitter_interface, "Respond\\Event\\EventEmitterInterface");
    REGISTER_INTERNAL_INTERFACE(respond_event_event_emitter_interface);
}
