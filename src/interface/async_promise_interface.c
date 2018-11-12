#include "respondphp.h"
#include "interface/async_promise_interface.h"

DECLARE_FUNCTION_ENTRY(respond_async_promise_interface) =
{
    PHP_ABSTRACT_ME(respond_async_promise_interface, then, ARGINFO(respond_async_promise_interface, then))
    PHP_ABSTRACT_ME(respond_async_promise_interface, catch, ARGINFO(respond_async_promise_interface, catch))
    PHP_ABSTRACT_ME(respond_async_promise_interface, finally, ARGINFO(respond_async_promise_interface, finally))
    PHP_ABSTRACT_ME(respond_async_promise_interface, resolve, ARGINFO(respond_async_promise_interface, resolve))
    PHP_ABSTRACT_ME(respond_async_promise_interface, reject, ARGINFO(respond_async_promise_interface, reject))
    PHP_ABSTRACT_ME(respond_async_promise_interface, cancel, NULL)
    PHP_FE_END
};

CLASS_ENTRY_FUNCTION_D(respond_async_promise_interface)
{
    INIT_CLASS(respond_async_promise_interface, "Respond\\Async\\PromiseInterface");
    REGISTER_INTERNAL_INTERFACE(respond_async_promise_interface);
    REGISTER_CLASS_CONSTANT_STRING(respond_async_promise_interface, PENDING, "pending");
    REGISTER_CLASS_CONSTANT_STRING(respond_async_promise_interface, FULFILLED, "fullfilled");
    REGISTER_CLASS_CONSTANT_STRING(respond_async_promise_interface, REJECTED, "rejected");
}
