#include "respondphp.h"
#include "version.h"
static rp_task_type_t rp_task_type = ACTOR;

uv_loop_t main_loop = {.data = NULL};
pid_t actor_pid;
uv_pipe_t ipc_pipe;
uv_pipe_t data_pipe;
uv_pipe_t routine_pipe;
zend_class_entry *rp_promise_ce = NULL;

static void declare_interfaces()
{
    CLASS_ENTRY_FUNCTION_C(respond_event_event_emitter_interface);
    CLASS_ENTRY_FUNCTION_C(respond_stream_server_interface);
    CLASS_ENTRY_FUNCTION_C(respond_stream_writable_stream_interface);
    CLASS_ENTRY_FUNCTION_C(respond_stream_readable_stream_interface);
    CLASS_ENTRY_FUNCTION_C(respond_stream_connection_interface);
    CLASS_ENTRY_FUNCTION_C(respond_socket_connector_interface);
    CLASS_ENTRY_FUNCTION_C(respond_async_promise_interface);
}

zend_module_entry respondphp_module_entry =
{
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "respondphp",
    NULL,
    PHP_MINIT(respondphp),
    PHP_MSHUTDOWN(respondphp),
    PHP_RINIT(respondphp),
    PHP_RSHUTDOWN(respondphp),
    PHP_MINFO(respondphp),
#if ZEND_MODULE_API_NO >= 20010901
    "0.1",
#endif
    STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL_RESPONDPHP
    ZEND_GET_MODULE(respondphp)
#endif

PHP_MINIT_FUNCTION(respondphp)
{
    declare_interfaces();
    CLASS_ENTRY_FUNCTION_C(respond_event_loop);
    CLASS_ENTRY_FUNCTION_C(respond_connector_pipe);
    CLASS_ENTRY_FUNCTION_C(respond_connector_tcp);
    CLASS_ENTRY_FUNCTION_C(respond_server_tcp);
    CLASS_ENTRY_FUNCTION_C(respond_server_udp);
    CLASS_ENTRY_FUNCTION_C(respond_server_pipe);
    CLASS_ENTRY_FUNCTION_C(respond_server_routine);
    CLASS_ENTRY_FUNCTION_C(respond_stream_connection);
    CLASS_ENTRY_FUNCTION_C(respond_network_resolver);
    CLASS_ENTRY_FUNCTION_C(respond_system_timer);
    CLASS_ENTRY_FUNCTION_C(respond_async_cancelable_promise);
    CLASS_ENTRY_FUNCTION_C(respond_server_http);
#ifdef HAVE_OPENSSL
    CLASS_ENTRY_FUNCTION_C(respond_connector_secure);
    CLASS_ENTRY_FUNCTION_C(respond_server_secure);
    CLASS_ENTRY_FUNCTION_C(respond_stream_secure);
#endif
    RP_ASSERT(main_loop.data == NULL);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(respondphp)
{
    return SUCCESS;
}

PHP_RINIT_FUNCTION(respondphp)
{
    zend_eval_string(PREDEFINED_PHP, NULL, "predefine php code");
    rp_promise_ce = rp_fetch_ce(ZEND_STRL(PREDEFINED_PHP_Respond_Async_Promise));
    rp_reactors_init();
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(respondphp)
{
    rp_reactors_destroy();
    return SUCCESS;
}

PHP_MINFO_FUNCTION(respondphp)
{
    char version_str[20];
    snprintf(version_str, sizeof(version_str), "%d.%d.%d %s", UV_VERSION_MAJOR, UV_VERSION_MINOR, UV_VERSION_PATCH, UV_VERSION_SUFFIX);
    php_info_print_table_start();
    php_info_print_table_header(2, "respondphp support", "enabled");  
    php_info_print_table_row(2, "respond php version", RESPOND_VERSION_STRING);
    php_info_print_table_row(2, "libuv version", version_str);
    php_info_print_table_end();
}

rp_task_type_t rp_get_task_type()
{
    return rp_task_type;
}

void rp_set_task_type(rp_task_type_t type)
{
    rp_task_type = type;
}

void rp_free_cb(void *data)
{
    rp_free(data);
}