#ifndef SEMANTIC_SYSTEM_DISPATCHER_DSL_HPP
#define SEMANTIC_SYSTEM_DISPATCHER_DSL_HPP

/// ========== [ Semantic Dispatch System DSL ] ==========
/// This system allows templated compile-time dispatch of all semantic actions,
/// providing a clean, declarative, zero-runtime-cost way to route operations.
///
/// Think: ss.call<"module.method">(args...)
///

#include <core/fixed_string.hpp>

#define NOP static_assert(true)

#define CALL_STR call_string

#define DISPATCH_DECLARE_HANDLER()                   \
    template<FixedString CALL_STR, typename... Args> \
    auto call(Args... args);                         \
    NOP // Absorbs trailing `;`.

#define DISPATCH_DEFINE_HANDLER_BEGIN(CLASS_NAME)    \
    template<FixedString CALL_STR, typename... Args> \
    auto CLASS_NAME::call(Args... args)              \
    {                                                \
        NOP // Absorbs trailing `;`.

#define DISPATCH_DEFINE_HANDLER_END(CLASS_NAME) \
    }                                           \
    NOP // Absorbs trailing `;`.

#define DISPATCH_BEGIN_CALLS() \
    if constexpr (false) {     \
        NOP // Absorbs trailing `;`.

#define DISPATCH_END_CALLS()             \
    }                                    \
    else UnknownCallStr<CALL_STR> dummy; \
    NOP // Absorbs trailing `;`.


#define _SELECT_METHOD_CALL(method_name)                    \
    }                                                       \
    else if constexpr (CALL_STR == #method_name)            \
    {                                                       \
        using return_type = decltype(method_name(args...)); \
        NOP // Absorbs trailing `;`.

#define _SELECT_MODULE_CALL(module_name)                                       \
    }                                                                          \
    else if constexpr (CALL_STR.starts_with(#module_name"."))                  \
    {                                                                          \
        constexpr auto subcall_str = CALL_STR.without_prefix(#module_name"."); \
        using return_type = decltype(module_name.call<subcall_str>(args...));  \
        NOP // Absorbs trailing `;`.

#define _FORWARD_METHOD_CALL(method_name)      \
    if constexpr (std::is_void_v<return_type>) \
        method_name(args...);                  \
    else                                       \
        return method_name(args...);           \
    NOP // Absorbs trailing `;`.

#define _FORWARD_MODULE_CALL(module_name)                                  \
    if constexpr (std::is_void_v<return_type>)                             \
        module_name.call<subcall_str>(args...);                            \
    else                                                                   \
        return module_name.call<subcall_str>(args...);                     \
    NOP // Absorbs trailing `;`.

#define _RETURN_IF_MAIN_DISPATCHER_IS_NOT_GOOD()                              \
    if (!MAIN_DISPATCHER_IS_GOOD())                                           \
    {                                                                         \
        if constexpr (std::is_void_v<return_type>)                            \
            return;                                                           \
        else if constexpr (std::is_pointer_v<return_type>)                    \
            return nullptr;                                                   \
        else if constexpr (std::is_default_constructible_v<return_type>)      \
            return return_type{};                                             \
        else                                                                  \
            static_assert(false, "Return type is not default-constructible"); \
    }

#if defined(MAIN_DISPATCHER) && defined(MAIN_DISPATCHER_IS_GOOD)
    #define DISPATCH_METHOD_CALL(method_name)     \
        _SELECT_METHOD_CALL(method_name);         \
        _RETURN_IF_MAIN_DISPATCHER_IS_NOT_GOOD(); \
        _FORWARD_METHOD_CALL(method_name);        \
        NOP // Absorbs trailing `;`.

    #define DISPATCH_MODULE_CALL(module_name)     \
        _SELECT_MODULE_CALL(module_name);         \
        _RETURN_IF_MAIN_DISPATCHER_IS_NOT_GOOD(); \
        _FORWARD_MODULE_CALL(module_name);        \
        NOP // Absorbs trailing `;`.

#elif !defined(MAIN_DISPATCHER) && !defined(MAIN_DISPATCHER_CONTROL_CODE)
    #define DISPATCH_METHOD_CALL(method_name) \
        _SELECT_METHOD_CALL(method_name);     \
        _FORWARD_METHOD_CALL(method_name);    \
        NOP // Absorbs trailing `;`.

    #define DISPATCH_MODULE_CALL(module_name) \
        _SELECT_MODULE_CALL(module_name);     \
        _FORWARD_MODULE_CALL(module_name);    \
        NOP // Absorbs trailing `;`.

#elif !defined(MAIN_DISPATCHER) && defined(MAIN_DISPATCHER_CONTROL_CODE)
    #define DISPATCH_METHOD_CALL(method_name)                                               \
        static_assert(false,                                                                \
        "`MAIN_DISPATCHER_IS_GOOD()` is defined but `MAIN_DISPATCHER` macro-flag is not!\n" \
        "Please remove definion!"                                                           \
        );                                                                                  \
        NOP // Absorbs trailing `;`.

    #define DISPATCH_MODULE_CALL(module_name)                                               \
        static_assert(false,                                                                \
        "`MAIN_DISPATCHER_IS_GOOD()` is defined but `MAIN_DISPATCHER` macro-flag is not!\n" \
        "Please remove definion!"                                                           \
        );                                                                                  \
        NOP // Absorbs trailing `;`.

#elif defined(MAIN_DISPATCHER) && !defined(MAIN_DISPATCHER_CONTROL_CODE)
    #define DISPATCH_METHOD_CALL(method_name)                                                   \
        static_assert(false,                                                                    \
            "`MAIN_DISPATCHER` macro-flag is defined but `MAIN_DISPATCHER_IS_GOOD()` is not!\n" \
            "Please define!"                                                                    \
        );                                                                                      \
        NOP // Absorbs trailing `;`.

    #define DISPATCH_MODULE_CALL(module_name)                                                   \
        static_assert(false,                                                                    \
            "`MAIN_DISPATCHER` macro-flag is defined but `MAIN_DISPATCHER_IS_GOOD()` is not!\n" \
            "Please define!"                                                                    \
        );                                                                                      \
        NOP // Absorbs trailing `;`.

#endif

#ifndef DISPATCH_METHOD_CALL
    #error "DISPATCH_METHOD_CALL macro was not defined — check MAIN_DISPATCHER / CONTROL_CODE defines."
#endif
#ifndef DISPATCH_MODULE_CALL
    #error "DISPATCH_MODULE_CALL macro was not defined — check MAIN_DISPATCHER / CONTROL_CODE defines."
#endif


template<FixedString unused_dummy> // we use this hack to cause error
struct UnknownCallStr
{
    // "\nUnknown call_str used in `call` dispatcher\n"
    // "(Look at the generated notes, to find the call_str that caused the error)"
    static_assert([] { return false; }(), // We circle with red emojis to hint ERROR
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
