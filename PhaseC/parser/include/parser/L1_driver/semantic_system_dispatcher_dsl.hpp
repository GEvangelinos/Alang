#ifndef SEMANTIC_SYSTEM_DISPATCHER_DSL_HPP
#define SEMANTIC_SYSTEM_DISPATCHER_DSL_HPP

/// ========== [ Semantic Dispatch System DSL ] ==========
/// This system allows templated compile-time dispatch of all semantic actions,
/// providing a clean, declarative, zero-runtime-cost way to route operations.
///
/// Think: ss.call<"module.method">(args...)
///

#include <core/fixed_string.hpp>

#define CALL_STR call_string
#define DISPATCHER_INVARIANTS\
    do\
       \
    {\
    }while (0)

#define DISPATCH_DECLARE_HANDLER()                   \
    template<FixedString CALL_STR, typename... Args> \
    auto call(Args... args);                         \
    static_assert(true) // Absorbs `;` from when calling the macro... ((void)0) can't be used in its call context.


#define DISPATCH_DEFINE_HANDLER_BEGIN(CLASS_NAME)         \
    template<FixedString CALL_STR, typename... Args>      \
    auto CLASS_NAME::call(Args... args)                   \
    {                                                     \
        ((void)0) //  Absorbs `;` from when calling the macro...

#define DISPATCH_DEFINE_HANDLER_END(CLASS_NAME) \
    }                                           \
    static_assert(true) //  Absorbs `;` from when calling the macro...
// (We can't use ((void)0) as it outside function (global, namespace, or class space)

#define DISPATCH_BEGIN_CALLS() \
    if constexpr (false) {     \
        ((void)0) //  Absorbs `;` from when calling the macro...

#define DISPATCH_END_CALLS()             \
    }                                    \
    else UnknownCallStr<CALL_STR> dummy; \
    ((void)0) //  Absorbs `;` from when calling the macro...

#define DISPATCH_CALL_METHOD(method_name)                             \
    }                                                                 \
    else if constexpr (CALL_STR == #method_name)                      \
    {                                                                 \
        if constexpr (std::is_void_v<decltype(method_name(args...))>) \
            method_name(args...);                                     \
        else                                                           \
            return method_name(args...);                              \
        ((void)0) //  Absorbs `;` from when calling the macro...

#define DISPATCH_CALL_MODULE(module_name)                                               \
    }                                                                                   \
    else if constexpr (CALL_STR.starts_with(#module_name"."))                           \
    {                                                                                   \
        constexpr auto subcall_str = CALL_STR.without_prefix(#module_name".");          \
        if constexpr (std::is_void_v<decltype(module_name.call<subcall_str>(args...))>) \
            module_name.call<subcall_str>(args...);                                     \
        else                                                                            \
            return module_name.call<subcall_str>(args...);                              \
        ((void)0) //  Absorbs `;` from when calling the macro...

template<FixedString unused_dummy> // we use this hack to cause error
struct UnknownCallStr
{
    // "\nUnknown call_str used in `call` dispatcher\n"
    // "(Look at the generated notes, to find the call_str that caused the error)"
    static_assert([] { return false; }(),
                  R"(
🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑
🛑======== [ DISPATCH DSL FAILURE ] ========🛑
🛑   Unknown call_str used in dispatcher.   🛑
🛑   Check for typos or missing handlers.   🛑
🛑   Take a look at the generated notes     🛑
🛑   below by the compiler. Look for:       🛑
🛑   UnknownCallStr<FixedString<>{"..."}>   🛑
🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑)"
    );
};

#endif // SEMANTIC_SYSTEM_DISPATCHER_DSL_HPP
