/**
 * Debug Tools Overview
 *
 * This header provides a unified set of diagnostic macros used throughout the compiler.
 *
 * These tools make control-flow contracts explicit, improve debug diagnostics,
 * and provide optimization hints to the compiler in release builds.
 */

#ifndef SUPPORT_DEBUG_TOOLS_HPP
#define SUPPORT_DEBUG_TOOLS_HPP

#include <iostream>
#include <limits>
#include "support/format_adapter.hpp"
#include "support/smart_assert.h"

#ifdef TO_STRING
#error "Macro collision detected"
#endif
#define TO_STRING(x) #x

#ifdef ATTACH_CONTEXT_ON_COMMAND
#error "Macro collision detected"
#endif
#define ATTACH_CONTEXT_ON_COMMAND(message, command)       \
        do                                                \
        {                                                 \
        std::cout << ATTACH_CONTEXT(message) << std::endl;\
        command;                                          \
        }while(0)

#ifdef ATTACH_CONTEXT
#error "Macro collision detected"
#endif
#define ATTACH_CONTEXT(message) \
        FMT::format("{}:{} -> {}(): {}", __FILENAME__, __LINE__, __func__, (message))

#ifdef ATTACH_CONTEXT_CT
#error "Macro collision detected"
#endif
#define ATTACH_CONTEXT_CT(message)(__FILENAME__ __LINE__ message)

#ifdef UNIMPLEMENTED
#error "Macro collision detected"
#endif
#define UNIMPLEMENTED(message_if_reached)                                            \
        do                                                                           \
        {                                                                            \
                throw std::logic_error(ATTACH_CONTEXT(FMT::format(                   \
                    "Control flow reached unimplemented code.\n"                     \
                    "User wrote: {}",                                                \
                    message_if_reached                                               \
                )));                                                                 \
        } while (0)

#if defined(__GNUC__) || defined(__clang__)
        #ifdef ALAWAYS_INLINE
        #error "Macro collision detected"
        #endif
        #define ALWAYS_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
        #ifdef ALAWAYS_INLINE
        #error "Macro collision detected"
        #endif
        #define ALWAYS_INLINE inline __forceinline
#else
        #ifdef ALAWAYS_INLINE
        #error "Macro collision detected"
        #endif
        #define ALWAYS_INLINE inline
#endif
#ifdef DEBUG_MODE
        #ifdef DEBUG
        #error "Macro collision detected"
        #endif
        #define DEBUG(code) code

        #ifdef DEBUG_ALAWAYS_INLINE
        #error "Macro collision detected"
        #endif
        #define DEBUG_ALWAYS_INLINE // Disable inlining to ensure that a function visible debug symbols.

        #ifdef DEBUG_NULLIFY
        #error "Macro collision detected"
        #endif
        #define DEBUG_NULLIFY(pointer) ((pointer) = nullptr)

//  Active in debug mode: evaluates and runs full SMART_ASSERT logic
        #ifdef DMASSERT
        #error "Macro collision detected"
        #endif
        #define DMASSERT(...) SMART_ASSERT(__VA_ARGS__)

//  Also active in debug mode: same as above, runs full logic
        #ifdef DMASSERT_EVAL
        #error "Macro collision detected"
        #endif
        #define DMASSERT_EVAL(...) SMART_ASSERT(__VA_ARGS__)
#else
        #ifdef DEBUG
        #error "Macro collision detected"
        #endif
        #define DEBUG(code) // Ignore input!


        #ifdef DEBUG_ALWAYS_INLINE
        #error "Macro collision detected"
        #endif
        #define DEBUG_ALWAYS_INLINE ALWAYS_INLINE

        #ifdef DEBUG_NULLIFY
        #error "Macro collision detected"
        #endif
        #define DEBUG_NULLIFY(pointer) ((void)0)

//  In release mode: disables assertion and also skips evaluating expressions (no side effects)
        #ifdef DMASSERT
        #error "Macro collision detected"
        #endif
        #define DMASSERT(...) ((void)0)

//  In release mode: disables assertion logic, but still evaluates expressions (preserves side effects)
        #ifdef DMASSERT_EVAL
        #error "Macro collision detected"
        #endif
        #define DMASSERT_EVAL(...) ((void)(__VA_ARGS__))
#endif

#ifdef REPORT_UNREACHABLE_VIOLATION
#error "Macro collision detected"
#endif
#define REPORT_UNREACHABLE_VIOLATION(_message_if_reached)                  \
        do                                                                 \
        {                                                                  \
                std::cerr << ATTACH_CONTEXT(FMT::format(                   \
                        "Unreachable Violation: {}", _message_if_reached)) \
                << std::endl;                                              \
        } while (0)

#if defined(__GNUC__) || defined(__clang__)
        #ifdef UNREACHABLE
        #error "Macro collision detected"
        #endif
        #define UNREACHABLE(_message_if_reached)                                   \
                do                                                                 \
                {                                                                  \
                        REPORT_UNREACHABLE_VIOLATION(_message_if_reached);         \
                        __builtin_unreachable();                                   \
                } while (0)

#elif defined(_MSC_VER)

        #ifdef UNREACHABLE
        #error "Macro collision detected"
        #endif
        #define UNREACHABLE(_message_if_reached)                           \
                do                                                         \
                {                                                          \
                        REPORT_UNREACHABLE_VIOLATION(_message_if_reached); \
                        __assume(false);                                   \
                } while (0)
#else
        #include <cstdlib>
        #ifdef UNREACHABLE
        #error "Macro collision detected"
        #endif
        #define UNREACHABLE(_message_if_reached)                           \
                do                                                         \
                {                                                          \
                        REPORT_UNREACHABLE_VIOLATION(_message_if_reached); \
                        std::abort();                                      \
                } while (0)
#endif

#ifdef DEBUG_MODE
        #define DEBUG_UNREACHABLE(...) UNREACHABLE(__VA_ARGS__)
#else
        #define DEBUG_UNREACHABLE(...)
#endif

namespace alpha::support
{
template<typename RangeType, typename NumType>
[[nodiscard]] bool
is_in_numeric_range(const NumType unsigned_number)
{
        // Clunky I know... but I don't use signed values, and I don't want to bother with
        // signed, unsigned conversion stuff.
        static_assert(std::is_unsigned_v<RangeType>);
        static_assert(std::is_unsigned_v<RangeType>);

        return unsigned_number >= std::numeric_limits<RangeType>::min() &&
               unsigned_number <= std::numeric_limits<RangeType>::max();
}
}

#endif // SUPPORT_DEBUG_TOOLS_HPP
