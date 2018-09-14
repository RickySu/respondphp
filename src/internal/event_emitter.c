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
    HashTable *event_hook = (HashTable *) getTraitResource();
    const char *event = NULL;
    size_t event_len;
    zval *params;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &params)) {
        return;
    }
    fprintf(stderr, "emit %.*s\n", event_len, event);
    RETURN_ZVAL(params, 1, 0);
}
