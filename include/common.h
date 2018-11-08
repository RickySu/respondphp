#ifndef _RP_COMMON_H
#define _RP_COMMON_H

#define CLASS_ENTRY(name) respondphp_ce_##name

#define OBJECT_HANDLER(name) object_handler_##name

#define IMPLEMENTS_ENTRY_FUNCTION_D(name) \
    void init_respondphp_implements_##name()

#define IMPLEMENTS_ENTRY_FUNCTION_C(name) \
    init_respondphp_implements_##name()

#define CLASS_ENTRY_FUNCTION_D(name) \
    void init_respondphp_ce_##name()

#define CLASS_ENTRY_FUNCTION_C(name) \
    init_respondphp_ce_##name()    

#define DECLARE_CLASS_ENTRY(name) \
    zend_object_handlers OBJECT_HANDLER(name); \
    zend_class_entry *CLASS_ENTRY(name); \
    CLASS_ENTRY_FUNCTION_D(name)

#define FUNCTION_ENTRY(name) \
    respondphp_fe_##name

#define DECLARE_FUNCTION_ENTRY(name) \
    static zend_function_entry FUNCTION_ENTRY(name)[]

#define REGISTER_INTERNAL_CLASS(name) \
    CLASS_ENTRY(name) = zend_register_internal_class_ex(&ce, NULL)

#define REGISTER_INTERNAL_INTERFACE(name) \
    CLASS_ENTRY(name) = zend_register_internal_interface(&ce)

#define REGISTER_INTERNAL_CLASS_EX(name, base) \
    CLASS_ENTRY(name) = zend_register_internal_class_ex(&ce, CLASS_ENTRY(base))

#define INIT_CLASS(name, ns_name) \
    zend_class_entry ce; \
    INIT_CLASS_ENTRY(ce, ns_name, FUNCTION_ENTRY(name))

#define INIT_CLASS_WITH_OBJECT_NEW(name, ns_name, create_function) \
    INIT_CLASS(name, ns_name); \
    memcpy(&OBJECT_HANDLER(name), zend_get_std_object_handlers(), sizeof(zend_object_handlers))

#define REGISTER_CLASS_WITH_OBJECT_NEW(name, ns_name, create_function) \
    INIT_CLASS_WITH_OBJECT_NEW(name, ns_name, create_function); \
    REGISTER_INTERNAL_CLASS(name); \
    CLASS_ENTRY(name)->create_object = create_function

#define EXTENDS_CLASS_WITH_OBJECT_NEW(name, create_function, base) \
    INIT_CLASS_WITH_OBJECT_NEW(name, create_function); \
    REGISTER_INTERNAL_CLASS_EX(name, base); \
    CLASS_ENTRY(name)->create_object = create_function    
 
#define REGISTER_CLASS(name, ns_name) \
    zend_class_entry ce; \
    INIT_CLASS_ENTRY(ce, ns_name, FUNCTION_ENTRY(name)); \
    REGISTER_INTERNAL_CLASS(name)
    
#define EXTENDS_CLASS(name, base) \
    zend_class_entry ce; \
    INIT_CLASS_ENTRY(ce, #name, FUNCTION_ENTRY(name)); \
    REGISTER_INTERNAL_CLASS_EX(name, base)

#define ARGINFO(classname, method) \
    arginfo_##classname##_##method

#define FETCH_RESOURCE(pointer, type) (type *) ((void *)pointer - XtOffsetOf(type, zo))

#define FETCH_RESOURCE_FROM_EXTEND(pointer, item, type) (type *) ((void *)pointer - XtOffsetOf(type, item))
    
#define FETCH_OBJECT_RESOURCE(object, type) FETCH_RESOURCE(Z_OBJ_P(object), type)

#define REGISTER_CLASS_CONSTANT_LONG(class, name) \
    zend_declare_class_constant_long(CLASS_ENTRY(class), ZEND_STRL(#name), name)

#define ALLOC_RESOURCE(x, ce) ecalloc(1, sizeof(x) + zend_object_properties_size(ce))

#define Z_DELREF_AND_DTOR_P(o) \
    do{ \
        if(Z_REFCOUNT_P(o) == 1){ \
            zval_dtor(o); \
        } \
        else{ \
            Z_DELREF_P(o); \
        }\
    }while(0)

#endif

#define IMPLEMENTS_INTERFACE(classname, interface) \
    do{ \
        zend_string *interface_##interface##_name = zend_string_init(ZEND_STRL(interface), 0); \
        zend_class_entry *interface_##interface##_ce = zend_fetch_class(interface_##interface##_name, ZEND_FETCH_CLASS_AUTO); \
        zend_class_implements(CLASS_ENTRY(classname), 1, interface_##interface##_ce); \
        zend_string_release(interface_##interface##_name); \
    } while(0)
#define getTraitResource() resource_field
#define TRAIT_FUNCTION_ARG_INFO(ce, trait_name) TRAIT_FUNCTION_ARG_INFO_##trait_name(ce)
#define TRAIT_METHOD_NAME(trait_name, method_name) rp_trait_##trait_name##_##method_name
#define TRAIT_FUNCTION_ENTRY_ME(ce, trait_name) TRAIT_FUNCTION_ENTRY_ME_##trait_name(ce)
#define TRAIT_PHP_METHOD(ce, trait_name) TRAIT_PHP_METHOD_##trait_name(ce)
#define TRAIT_PHP_METHOD_USE(ce, trait_name, resource_type, resource_field) TRAIT_PHP_METHOD_USE_##trait_name(ce, resource_type, resource_field)
#define TRAIT_PHP_METHOD_DEFINE(trait_name, method_name) TRAIT_METHOD_NAME(trait_name, method_name)(INTERNAL_FUNCTION_PARAMETERS, void *resource_field)
#define TRAIT_PHP_METHOD_PASSTHRU(ce, trait_name, method_name, resource_type, resource_field) \
PHP_METHOD(ce, method_name) \
{ \
    resource_type *resource = FETCH_OBJECT_RESOURCE(getThis(), resource_type); \
    TRAIT_METHOD_NAME(trait_name, method_name)(INTERNAL_FUNCTION_PARAM_PASSTHRU, (void *)&resource->resource_field); \
}

#if PHP_VERSION_ID >= 70200
    #define RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO
    #define RP_BEGIN_ARG_INFO(name, type, allow_null) ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(name, type, allow_null)
#else
    #define RP_BEGIN_ARG_WITH_RETURN_OBJ_INFO(name, class_name, allow_null) ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(name, IS_OBJECT, class_name, allow_null)
    #define RP_BEGIN_ARG_INFO(name, type, allow_null) ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(name, type, NULL, allow_null)
#endif

#define RP_END_ARG_INFO ZEND_END_ARG_INFO