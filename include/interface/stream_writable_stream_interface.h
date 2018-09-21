#ifndef _RP_STREAM_WRITABLE_STREAM_INTERFACE_H
#define _RP_STREAM_WRITABLE_STREAM_INTERFACE_H

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface_isWritable(ce) \
    RP_BEGIN_ARG_INFO(ARGINFO(ce, isWritable), _IS_BOOL, 0) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface_write(ce) \
    RP_BEGIN_ARG_INFO(ARGINFO(ce, write), _IS_BOOL, 0) \
        ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface_end(ce) \
    ZEND_BEGIN_ARG_INFO(ARGINFO(ce, end), 0) \
        ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 1) \
    ZEND_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface_close(ce) \
    ZEND_BEGIN_ARG_INFO(ARGINFO(ce, close), 0) \
    RP_END_ARG_INFO()

#define TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface(ce) \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface_isWritable(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface_write(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface_end(ce); \
    TRAIT_FUNCTION_ARG_INFO_respond_stream_writable_stream_interface_close(ce)

TRAIT_FUNCTION_ARG_INFO(respond_stream_writable_stream_interface, respond_stream_writable_stream_interface);
DECLARE_FUNCTION_ENTRY(respond_stream_writable_stream_interface) =
{
    PHP_ABSTRACT_ME(respond_stream_writable_stream_interface, isWritable, ARGINFO(respond_stream_writable_stream_interface, isWritable))
    PHP_ABSTRACT_ME(respond_stream_writable_stream_interface, write, ARGINFO(respond_stream_writable_stream_interface, write))
    PHP_ABSTRACT_ME(respond_stream_writable_stream_interface, end, ARGINFO(respond_stream_writable_stream_interface, end))
    PHP_ABSTRACT_ME(respond_stream_writable_stream_interface, close, ARGINFO(respond_stream_writable_stream_interface, close))
    PHP_FE_END
};
#endif
