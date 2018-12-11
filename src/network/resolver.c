#include "respondphp.h"
#include "network/resolver.h"

static void on_addrinfo_resolved(rp_resolver_into_t *info, int status, struct addrinfo *res);
static void on_nameinfo_resolved(rp_resolver_into_t *info, int status, const char *hostname, const char *service);
static void getaddrinfo_async_cb(rp_resolver_into_t *info);
static void getnameinfo_async_cb(rp_resolver_into_t *info);

DECLARE_FUNCTION_ENTRY(respond_network_resolver) =
{
    PHP_ME(respond_network_resolver, __construct, NULL, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
    PHP_ME(respond_network_resolver, getaddrinfo, ARGINFO(respond_network_resolver, getaddrinfo), ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(respond_network_resolver, getnameinfo, ARGINFO(respond_network_resolver, getnameinfo), ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_network_resolver)
{
    REGISTER_CLASS(respond_network_resolver, "Respond\\Network\\Resolver");
}

static void on_nameinfo_resolved(rp_resolver_into_t *info, int status, const char *hostname, const char *service)
{
    zval result;
    if(status >= 0) {
        ZVAL_STRING(&result, hostname);
        rp_resolve_promise(&info->promise, &result);
        ZVAL_PTR_DTOR(&info->promise);
        ZVAL_PTR_DTOR(&result);
    }
    else{
        rp_reject_promise_long(&info->promise, status);
    }
    rp_free(info);
}

static void on_addrinfo_resolved(rp_resolver_into_t *info, int status, struct addrinfo *res)
{
    char addr_str[INET6_ADDRSTRLEN + 1];
    zval result, v4result, v6result;

    if(status != 0){
        rp_reject_promise_long(&info->promise, status);
        rp_free(info);
        return;
    }

    array_init(&result);
    array_init(&v4result);
    array_init(&v6result);

    while(res) {
        switch (res->ai_family) {
            case AF_INET:
                uv_ip4_name((struct sockaddr_in *) res->ai_addr, addr_str, sizeof(addr_str));
                add_next_index_string(&v4result, addr_str);
                break;
            case AF_INET6:
                uv_ip6_name((struct sockaddr_in *) res->ai_addr, addr_str, sizeof(addr_str));
                add_next_index_string(&v6result, addr_str);
                break;
        }
        res = res->ai_next;
    }

    add_assoc_zval(&result, "IPv4", &v4result);
    add_assoc_zval(&result, "IPv6", &v6result);
    rp_resolve_promise(&info->promise, &result);
    ZVAL_PTR_DTOR(&info->promise);
    ZVAL_PTR_DTOR(&result);
    rp_free(info);
    uv_freeaddrinfo(res);
}

static void getaddrinfo_async_cb(rp_resolver_into_t *info)
{
    int err;
    if((err = uv_getaddrinfo(&main_loop, (uv_getaddrinfo_t *) info, (uv_getaddrinfo_cb) on_addrinfo_resolved, info->info.addr.hostname->val, NULL, &info->info.addr.hints)) != 0) {
        rp_reject_promise_long(&info->promise, err);
        rp_free(info);
    }
    zend_string_release(&info->info.addr.hostname);
}

PHP_METHOD(respond_network_resolver, getaddrinfo)
{
    rp_resolver_into_t *info;

    info = rp_calloc(1, sizeof(rp_resolver_into_t));
    rp_make_promise_object(&info->promise);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &info->info.addr.hostname)) {
        rp_free(info);
        ZVAL_PTR_DTOR(&info->promise);
        return;
    }

    info->info.addr.hints.ai_family = PF_UNSPEC;
    info->info.addr.hints.ai_socktype = SOCK_DGRAM;
    RETVAL_ZVAL(&info->promise, 1, 0);
    zend_string_addref(&info->info.addr.hostname);
    rp_reactor_async_init((rp_reactor_async_init_cb) getaddrinfo_async_cb, info);
}

static void getnameinfo_async_cb(rp_resolver_into_t *info)
{
    int err;
    if(err = uv_getnameinfo(&main_loop, (uv_getnameinfo_t *) info, (uv_getnameinfo_cb) on_nameinfo_resolved, (const struct sockaddr *) &info->info.name.addr, 0)){
        rp_reject_promise_long(&info->promise, err);
        rp_free(info);
    }
}

PHP_METHOD(respond_network_resolver, getnameinfo)
{
    int err = 0;
    zend_string *ip_address;
    rp_resolver_into_t *info;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &ip_address)) {
        return;
    }

    info = rp_calloc(1, sizeof(rp_resolver_into_t));
    rp_make_promise_object(&info->promise);
    RETVAL_ZVAL(&info->promise, 1, 0);

    if(memchr(ip_address->val, ':', ip_address->len) == NULL) {
        err = uv_ip4_addr(ip_address->val, 0, &info->info.name.addr.sockaddr);
    }
    else {
        err = uv_ip6_addr(ip_address->val, 0, &info->info.name.addr.sockaddr6);
    }

    if(err != 0){
        rp_reject_promise_long(&info->promise, err);
        rp_free(info);
        return;
    }

    rp_reactor_async_init((rp_reactor_async_init_cb) getnameinfo_async_cb, info);
}

PHP_METHOD(respond_network_resolver, __construct)
{

}