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
#define DISPATCH_TARGET restricted_

#define DISPATCH_DEFINE_HANDLER_BEGIN()              \
    template<FixedString CALL_STR, typename... Args> \
    auto call(Args... args)                          \
    {                                                \
        if constexpr (false)                         \
        {                                            \
            NOP // Absorbs trailing `;`.

#define DISPATCH_DEFINE_HANDLER_END()        \
        }                                    \
        else UnknownCallStr<CALL_STR> dummy; \
    }                                        \
    NOP // Absorbs trailing `;`.



#define _SELECT_INTERNAL_METHOD_CALL(method_name)                           \
    }                                                                       \
    else if constexpr (CALL_STR == #method_name)                            \
    {                                                                       \
        using return_type = decltype(DISPATCH_TARGET.method_name(args...)); \
        NOP // Absorbs trailing `;`.

#define _SELECT_METHOD_CALL(method_name)                    \
    }                                                       \
    else if constexpr (CALL_STR == #method_name)            \
    {                                                       \
        using return_type = decltype(method_name(args...)); \
        NOP // Absorbs trailing `;`.

#define _SELECT_INTERNAL_MODULE_CALL(module_name)                                             \
    }                                                                                         \
    else if constexpr (CALL_STR.starts_with(#module_name"."))                                 \
    {                                                                                         \
        constexpr auto subcall_str = CALL_STR.without_prefix(#module_name".");                \
        using return_type = decltype(DISPATCH_TARGET.module_name.call<subcall_str>(args...)); \
        NOP // Absorbs trailing `;`.

#define _SELECT_MODULE_CALL(module_name)                                       \
    }                                                                          \
    else if constexpr (CALL_STR.starts_with(#module_name"."))                  \
    {                                                                          \
        constexpr auto subcall_str = CALL_STR.without_prefix(#module_name"."); \
        using return_type = decltype(module_name.call<subcall_str>(args...));  \
        NOP // Absorbs trailing `;`.

#define _FORWARD_INTERNAL_METHOD_CALL(method_name)   \
    if constexpr (std::is_void_v<return_type>)       \
        DISPATCH_TARGET.method_name(args...);        \
    else                                             \
        return DISPATCH_TARGET.method_name(args...); \
    NOP // Absorbs trailing `;`.

#define _FORWARD_METHOD_CALL(method_name)      \
    if constexpr (std::is_void_v<return_type>) \
        method_name(args...);                  \
    else                                       \
        return method_name(args...);           \
    NOP // Absorbs trailing `;`.

#define _FORWARD_INTERNAL_MODULE_CALL(module_name)                     \
    if constexpr (std::is_void_v<return_type>)                         \
        DISPATCH_TARGET.module_name.call<subcall_str>(args...);        \
    else                                                               \
        return DISPATCH_TARGET.module_name.call<subcall_str>(args...); \
    NOP // Absorbs trailing `;`.

#define _FORWARD_MODULE_CALL(module_name)              \
    if constexpr (std::is_void_v<return_type>)         \
        module_name.call<subcall_str>(args...);        \
    else                                               \
        return module_name.call<subcall_str>(args...); \
    NOP // Absorbs trailing `;`.

/// Ensures safe and consistent return type deduction in dispatcher logic.
///
/// In particular, for pointer-returning handlers (e.g., `const Expr*`),
/// we explicitly cast `nullptr` to `return_type` to avoid type mismatch issues.
/// Without the cast, `nullptr` would be deduced as `std::nullptr_t`, which
/// conflicts with `auto` return types when the dispatcher infers the return
/// type from `_SELECT_METHOD_CALL(...)`.
///
/// Placed immediately before `_FORWARD_METHOD_CALL`, this macro ensures that
/// all control paths return a value of the same deduced type — a requirement
/// for `auto` return functions.
#define _RETURN_IF_MAIN_DISPATCHER_IS_NOT_GOOD()                              \
    if (!good())                                                              \
    {                                                                         \
        if constexpr (std::is_void_v<return_type>)                            \
            return;                                                           \
        else if constexpr (std::is_pointer_v<return_type>)                    \
            return static_cast<return_type>(nullptr);                         \
        else if constexpr (std::is_default_constructible_v<return_type>)      \
            return return_type{};                                             \
        else                                                                  \
            static_assert(false, "Return type is not default-constructible"); \
    }

#define DISPATCH_MASTER_METHOD_CALL(method_name) \
    _SELECT_METHOD_CALL(method_name);            \
    _RETURN_IF_MAIN_DISPATCHER_IS_NOT_GOOD();    \
    _FORWARD_METHOD_CALL(method_name);           \
    NOP // Absorbs trailing `;`.

#define DISPATCH_MASTER_MODULE_CALL(module_name) \
    _SELECT_MODULE_CALL(module_name);            \
    _RETURN_IF_MAIN_DISPATCHER_IS_NOT_GOOD();    \
    _FORWARD_MODULE_CALL(module_name);           \
    NOP // Absorbs trailing `;`.

#define DISPATCH_SLAVE_METHOD_CALL(method_name) \
    _SELECT_INTERNAL_METHOD_CALL(method_name);  \
    _FORWARD_INTERNAL_METHOD_CALL(method_name); \
    NOP // Absorbs trailing `;`.

#define DISPATCH_SLAVE_MODULE_CALL(module_name) \
    _SELECT_INTERNAL_MODULE_CALL(module_name);  \
    _FORWARD_INTERNAL_MODULE_CALL(module_name); \
    NOP // Absorbs trailing `;`.

/// This templated struct is an intentional hack 😄
/// It's designed to be instantiated in the final `else` branch
/// of a constexpr if/else chain, when no valid match is found.
///
/// If all `if constexpr` conditions fail, and an unknown or incorrect
/// call_str is provided, this struct gets instantiated—triggering a
/// compile-time error with a clear diagnostic message.
///
/// Used to signal that a dispatcher call was made with an unrecognized key.
/// We also circle the static_assert() error message with RED emojis to be eye catching and hint ERROR.
template<FixedString unused_dummy>
struct UnknownCallStr
{
    // "\nUnknown call_str used in `call` dispatcher\n"
    // "(Look at the generated notes, to find the call_str that caused the error)"
    static_assert([]() constexpr { return false; }(),
                  R"(
🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑
🛑======== [ DISPATCH DSL FAILURE ] ========🛑
🛑   Unknown call_str used in dispatcher.   🛑
🛑   Check for:                             🛑
🛑       * Typos.                           🛑
🛑       * Routing through correct module.  🛑
🛑       * Selected module has specified    🛑
🛑         method declared.                 🛑
🛑   Take a look at the generated notes     🛑
🛑   below by the compiler. Look for:       🛑
🛑   UnknownCallStr<FixedString<>{"..."}>   🛑
🛑   to find call_str causing the error     🛑
🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑🛑)"
    );
};

#endif // SEMANTIC_SYSTEM_DISPATCHER_DSL_HPP
