#ifndef _RP_STREAM_READABLE_STREAM_INTERFACE_H
#define _RP_STREAM_READABLE_STREAM_INTERFACE_H

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_readable_stream_interface_isReadable(ce) \
    RP_BEGIN_ARG_INFO(ARGINFO(ce, isReadable), _IS_BOOL, 1) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_readable_stream_interface_close(ce) \
    ZEND_BEGIN_ARG_INFO(ARGINFO(ce, close), 0) \
    RP_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_readable_stream_interface(ce) \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_readable_stream_interface_isReadable(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_readable_stream_interface_close(ce)

TRAIT_FUNCTION_ARG_INFO(respond_stream_readable_stream_interface, respond_stream_readable_stream_interface);
DECLARE_FUNCTION_ENTRY(respond_stream_readable_stream_interface) =
{
    PHP_ABSTRACT_ME(respond_stream_readable_stream_interface, isReadable, ARGINFO(respond_stream_readable_stream_interface, isReadable))
    PHP_ABSTRACT_ME(respond_stream_readable_stream_interface, close, ARGINFO(respond_stream_readable_stream_interface, close))
    PHP_FE_END
};
#endif
