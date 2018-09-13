#include "respondphp.h"
#include "internal/event_emitter.h"

TRAIT_PHP_METHOD_DEFINE(event_emitter, on)
{

}

TRAIT_PHP_METHOD_DEFINE(event_emitter, off)
{

}

TRAIT_PHP_METHOD_DEFINE(event_emitter, removeListeners)
{

}

TRAIT_PHP_METHOD_DEFINE(event_emitter, getListeners)
{

}

TRAIT_PHP_METHOD_DEFINE(event_emitter, emit)
{
    fprintf(stderr, "emit\n");
    RETURN_LONG(100);
}
