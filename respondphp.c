#include "respondphp.h"
#include "version.h"
uv_loop_t main_loop;

static void implements_interfaces()
{
    CLASS_ENTRY_FUNCTION_C(respond_event_event_emitter_interface);
    CLASS_ENTRY_FUNCTION_C(respond_stream_writable_stream_interface);
    CLASS_ENTRY_FUNCTION_C(respond_stream_readable_stream_interface);
    CLASS_ENTRY_FUNCTION_C(respond_socket_connection_interface);
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
    implements_interfaces();
    CLASS_ENTRY_FUNCTION_C(respond_event_loop);
    CLASS_ENTRY_FUNCTION_C(respond_server_tcp);
    CLASS_ENTRY_FUNCTION_C(respond_connection_connection);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(respondphp)
{
    return SUCCESS;
}

PHP_RINIT_FUNCTION(respondphp)
{
//    zend_eval_string(PREDEFINED_PHP, NULL, "predefine php code");
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(respondphp)
{
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
