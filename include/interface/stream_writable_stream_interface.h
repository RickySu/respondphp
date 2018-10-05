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
    RP_BEGIN_ARG_INFO(ARGINFO(ce, end), _IS_BOOL, 0) \
        ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0) \
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
#endif
