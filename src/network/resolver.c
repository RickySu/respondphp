#include "respondphp.h"
#include "network/resolver.h"

static zend_object *create_respond_network_resolver_resource(zend_class_entry *ce);
static void free_respond_network_resolver_resource(zend_object *object);
static void on_addrinfo_resolved(rp_getaddrinto_ext_t *info, int status, struct addrinfo *res);
static void reject_result(rp_getaddrinto_ext_t *info, int err);

DECLARE_FUNCTION_ENTRY(respond_network_resolver) =
{
    PHP_ME(respond_network_resolver, getaddrinfo, ARGINFO(respond_network_resolver, getaddrinfo), ZEND_ACC_PUBLIC)
    PHP_ME(respond_network_resolver, getnameinfo, ARGINFO(respond_network_resolver, getnameinfo), ZEND_ACC_PUBLIC)
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_network_resolver)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_network_resolver, "Respond\\Network\\Resolver", create_respond_network_resolver_resource);
    OBJECT_HANDLER(respond_network_resolver).offset = XtOffsetOf(rp_network_resolver_ext_t, zo);
    OBJECT_HANDLER(respond_network_resolver).clone_obj = NULL;
    OBJECT_HANDLER(respond_network_resolver).free_obj = free_respond_network_resolver_resource;
}

static zend_object *create_respond_network_resolver_resource(zend_class_entry *ce)
{
    rp_network_resolver_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_network_resolver_ext_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);
    resource->zo.handlers = &OBJECT_HANDLER(respond_network_resolver);
    return &resource->zo;
}

static void free_respond_network_resolver_resource(zend_object *object)
{
    rp_network_resolver_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_network_resolver_ext_t);
    zend_object_std_dtor(object);
}

static void on_addrinfo_resolved(rp_getaddrinto_ext_t *info, int status, struct addrinfo *res)
{
    int addrlen = INET6_ADDRSTRLEN + 6;
    char addr_str[INET6_ADDRSTRLEN + 6];
    zval result, v4result, v6result;

    if(status != 0){
        reject_result(info, status);
        rp_free(info);
        return;
    }

    array_init(&result);
    array_init(&v4result);
    array_init(&v6result);

    while(res) {
        switch (res->ai_family) {
            case AF_INET:
                uv_ip4_name((struct sockaddr_in *) res->ai_addr, addr_str, addrlen);
                add_next_index_string(&v4result, addr_str);
                break;
            case AF_INET6:
                uv_ip6_name((struct sockaddr_in *) res->ai_addr, addr_str, addrlen);
                add_next_index_string(&v6result, addr_str);
                break;
        }
        fprintf(stderr, "res %p resolve: %s %s\n", res, addr_str, res->ai_canonname);
        res = res->ai_next;
    }

    add_assoc_zval(&result, "v4", &v4result);
    add_assoc_zval(&result, "v6", &v6result);
    rp_resolve_promise(&info->promise, &result);
    ZVAL_PTR_DTOR(&info->promise);
    ZVAL_PTR_DTOR(&result);
    zend_object_ptr_dtor(info->zo);
    rp_free(info);
}

PHP_METHOD(respond_network_resolver, getaddrinfo)
{
    int err;
    zval *self = getThis();
    rp_network_resolver_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_network_resolver_ext_t);
    zend_string *hostname;
    rp_getaddrinto_ext_t *info;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &hostname)) {
        return;
    }

    info = rp_calloc(1, sizeof(rp_getaddrinto_ext_t));
    rp_make_promise_object(&info->promise);

    info->hints.ai_family = PF_UNSPEC;
    info->hints.ai_socktype = SOCK_DGRAM;

    if((err = uv_getaddrinfo(&main_loop, (uv_getaddrinfo_t *) info, (uv_getaddrinfo_cb) on_addrinfo_resolved, hostname->val, NULL, &info->hints)) != 0) {
        RETVAL_ZVAL(&info->promise, 1, 0);
        reject_result(info, err);
        rp_free(info);
        return;
    }

    Z_ADDREF_P(self);
    info->zo = &resource->zo;
    RETVAL_ZVAL(&info->promise, 1, 0);
}

PHP_METHOD(respond_network_resolver, getnameinfo)
{

}

static void reject_result(rp_getaddrinto_ext_t *info, int err)
{
    zval exception;
    ZVAL_LONG(&exception, err);
    object_init_ex(&exception, zend_ce_exception);
    zend_call_method_with_1_params(&exception, NULL, &Z_OBJCE(exception)->constructor, "__construct", NULL, &exception);
    rp_reject_promise(&info->promise, &exception);
}