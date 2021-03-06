#include "respondphp.h"
#include "internal/event_emitter.h"

static void rp_event_hook_cache_list_free(zval *hook);
static void rp_event_hook_memory_free(zval *hook);

void rp_event_emitter_on(event_hook_t *event_hook, const char *event, size_t event_len, zval *hook)
{
    zval new_array, *array;
    fcall_info_t *fci;
    ht_counter_t *ht_counter;
    zend_ulong index;

    if((array = zend_hash_str_find(Z_ARRVAL_P(&event_hook->hook), event, event_len)) == NULL) {
        array_init(&new_array);
        add_assoc_zval_ex(&event_hook->hook, event, event_len, &new_array);
        array = &new_array;
    }

    index = zend_hash_next_free_element(Z_ARRVAL_P(array));
    zval_add_ref(hook);
    add_index_zval(array, index, hook);

    if((array = zend_hash_str_find(&event_hook->hook_cache, event, event_len)) == NULL) {
        ht_counter = rp_calloc(1, sizeof(ht_counter_t));
        zend_hash_init(&ht_counter->ht, 5, NULL, rp_event_hook_memory_free, 0);
        zend_hash_str_add_new_ptr(&event_hook->hook_cache, event, event_len, &ht_counter->ht);
    }
    else {
        ht_counter = Z_PTR_P(array);
    }

    fci =rp_malloc(sizeof(fcall_info_t));
    zend_fcall_info_init(hook, 0, &fci->fci, &fci->fcc, NULL, NULL);

    zend_hash_index_add_ptr(&ht_counter->ht, index, fci);
}

void rp_event_emitter_off(event_hook_t *event_hook, const char *event, size_t event_len, zval *hook)
{
    zval *array;
    zval fn;
    zval retval;
    zval params[3];
    ht_counter_t *ht_counter;

    fcall_info_t fci;

    if((array = zend_hash_str_find(&event_hook->hook_cache, event, event_len)) == NULL) {
        return;
    }

    ht_counter = Z_PTR_P(array);

    if((array = zend_hash_str_find(Z_ARRVAL_P(&event_hook->hook), event, event_len)) == NULL){
        return;
    }
    
    ZVAL_STRING(&fn, "array_search");
    zend_fcall_info_init(&fn, 0, FCI_PARSE_PARAMETERS_CC(fci), NULL, NULL);
    ZVAL_PTR_DTOR(&fn);
    ZVAL_COPY_VALUE(&params[0], hook);
    ZVAL_COPY_VALUE(&params[1], array);
    ZVAL_BOOL(&params[2], 1);
    fci_call_function(&fci, &retval, 3, params);
    if(Z_TYPE_P(&retval) != IS_LONG){
        return;
    }

    zend_hash_index_del(Z_ARRVAL_P(array), Z_LVAL(retval));
    zend_hash_index_del(&ht_counter->ht, Z_LVAL(retval));
}

void rp_event_emitter_removeListeners_internal(event_hook_t *event_hook, const char *event, size_t event_len)
{
    zend_hash_str_del(Z_ARRVAL_P(&event_hook->hook), event, event_len);
    zend_hash_str_del(&event_hook->hook_cache, event, event_len);
}

void rp_event_emitter_removeListeners(event_hook_t *event_hook, const char *event, size_t event_len)
{
    zend_hash_str_del(Z_ARRVAL_P(&event_hook->hook), event, event_len);
    zend_hash_str_del(&event_hook->hook_cache, event, event_len);
}

zval *rp_event_emitter_getListeners(event_hook_t *event_hook, const char *event, size_t event_len)
{
    return zend_hash_str_find(Z_ARRVAL_P(&event_hook->hook), event, event_len);
}

void rp_event_emitter_on_intrenal_ex(event_hook_t *event_hook, const char *event, size_t event_len, rp_event_emitter_internal_cb callback, void *data)
{
    zval *array;
    HashTable *ht;
    ht_internal_cb_t *internal_cb;

    if((array = zend_hash_str_find(&event_hook->internal_hook, event, event_len)) == NULL) {
        ht = rp_calloc(1, sizeof(HashTable));
        zend_hash_init(ht, 5, NULL, rp_event_hook_memory_free, 0);
        zend_hash_str_add_new_ptr(&event_hook->internal_hook, event, event_len, ht);
    }
    else {
        ht = Z_PTR_P(array);
    }

    internal_cb = rp_malloc(sizeof(ht_internal_cb_t));
    internal_cb->callback = callback;
    internal_cb->data = data;
    zend_hash_next_index_insert_ptr(ht, internal_cb);
}

void rp_event_emitter_emit_internal(event_hook_t *event_hook, const char *event, size_t event_len, int n_param, zval *param)
{
    zval *array, *current;
    HashTable *ht;
    ht_internal_cb_t *internal_cb;

    if((array = zend_hash_str_find(&event_hook->internal_hook, event, event_len)) != NULL) {
        ht = Z_PTR_P(array);

        zend_hash_internal_pointer_reset(ht);
        while ((current = zend_hash_get_current_data(ht)) != NULL) {
            internal_cb = Z_PTR_P(current);
            internal_cb->callback(n_param, param, internal_cb->data);
            zend_hash_move_forward(ht);
        }
    }

    rp_event_emitter_emit(event_hook, event, event_len, n_param, param);
}

void rp_event_emitter_emit(event_hook_t *event_hook, const char *event, size_t event_len, int n_param, zval *param)
{
    zval *array, *current, retval;
    ht_counter_t *ht_counter;
    fcall_info_t *fci;
    if((array = zend_hash_str_find(&event_hook->hook_cache, event, event_len)) == NULL) {
        return;
    }
    ht_counter = Z_PTR_P(array);

    zend_hash_internal_pointer_reset(&ht_counter->ht);
    while((current = zend_hash_get_current_data(&ht_counter->ht)) != NULL) {
        fci = Z_PTR_P(current);
        fci_call_function(fci, &retval, n_param, param);
        zend_hash_move_forward(&ht_counter->ht);
    }
}

static void rp_event_hook_memory_free(zval *hook)
{
    rp_free(Z_PTR_P(hook));
}

static void rp_event_hook_cache_list_free(zval *hook)
{
    HashTable *ht = Z_PTR_P(hook);
    zend_hash_destroy(ht);
    rp_free(ht);
}

void rp_event_hook_init(event_hook_t *hook)
{
    zend_hash_init(&hook->hook_cache, 5, NULL, rp_event_hook_cache_list_free, 0);
    zend_hash_init(&hook->internal_hook, 5, NULL, rp_event_hook_cache_list_free, 0);
    array_init(&hook->hook);
}

void rp_event_hook_destroy(event_hook_t *hook)
{
    zend_hash_destroy(&hook->hook_cache);
    zend_hash_destroy(&hook->internal_hook);
    ZVAL_PTR_DTOR(&hook->hook);
}
