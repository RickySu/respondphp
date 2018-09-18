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
    event_hook_t *event_hook = (event_hook_t *) getTraitResource();
    const char *event = NULL;
    size_t event_len;
    zval *params;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &event, &event_len, &params)) {
        return;
    }
    fprintf(stderr, "emit %.*s\n", event_len, event);
    RETURN_ZVAL(params, 1, 0);
}

static void rp_event_hook_cache_free(zval *hook)
{
    efree(Z_PTR_P(hook));
}

void rp_event_hook_init(event_hook_t *hook)
{
    zend_hash_init(&hook->hook_cache, 5, NULL, rp_event_hook_cache_free, 0);
    array_init(&hook->hook);
}

void rp_event_hook_destroy(event_hook_t *hook)
{
    zend_hash_destroy(&hook->hook_cache);
    ZVAL_PTR_DTOR(&hook->hook);
}

void rp_event_hook_add(event_hook_t *hook, zval *value)
{

}

void rp_event_hook_del(event_hook_t *hook, zval *value)
{
    
}