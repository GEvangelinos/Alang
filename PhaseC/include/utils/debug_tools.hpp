//
// Created by stygian on 5/25/25.
//

#ifndef DEBUG_TOOLS_HPP
#define DEBUG_TOOLS_HPP
#include "misc.hpp"
#include "smart_assert.h"

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif /* __FILENAME__ */

#ifdef DEBUG_MODE
        #define DEBUG_NULLIFY(pointer) ((pointer) = nullptr)
#else
        #define DEBUG_NULLIFY(pointer) ((void)0)
#endif

#define UNIMPLEMENTED()                                                           \
        do                                                                        \
        {                                                                         \
                throw std::logic_error(ATTACH_CONTEXT("Unimplemented function")); \
        } while (0)

#define ATTACH_CONTEXT(message) \
        FMT::format("{}:{} -> {}(): {}", __FILENAME__, __LINE__, __func__, message)

#ifdef DEBUG_MODE
//  Active in debug mode: evaluates and runs full SMART_ASSERT logic
        #define DEBUG_SMART_ASSERT(...) SMART_ASSERT(__VA_ARGS__)
//  Also active in debug mode: same as above, runs full logic
        #define DEBUG_SMART_ASSERT_EVAL(...) SMART_ASSERT(__VA_ARGS__)
#else
//  In release mode: disables assertion logic, but still evaluates expressions (preserves side effects)
        #define DEBUG_SMART_ASSERT_EVAL(...) ((void)(__VA_ARGS__))
//  In release mode: disables assertion and also skips evaluating expressions (no side effects)
        #define DEBUG_SMART_ASSERT(...) ((void)0)
#endif // DEBUG_MODE

#define REPORT_UNREACHABLE_VIOLATION(_message_if_reached)                                   \
        do                                                                                  \
        {                                                                                   \
                std::cerr << FMT::format("{}:{} | {}() --> Unreachable Violation: {}",      \
                                         __FILE__, __LINE__, __func__, _message_if_reached) \
                          << std::endl;                                                     \
        } while (0)

#ifdef DEBUG_MODE
        #define DEBUG_ALWAYS_INLINE // Disable inlining to ensure that a function visible debug symbols.
#else
        #define DEBUG_ALWAYS_INLINE ALWAYS_INLINE
#endif

#if defined(__GNUC__) || defined(__clang__)
        #define UNREACHABLE(_message_if_reached)                           \
                do                                                         \
                {                                                          \
                        REPORT_UNREACHABLE_VIOLATION(_message_if_reached); \
                        __builtin_unreachable();                           \
                } while (0)
#elif defined(_MSC_VER)
        #define UNREACHABLE(_message_if_reached)                           \
                do                                                         \
                {                                                          \
                        REPORT_UNREACHABLE_VIOLATION(_message_if_reached); \
                        __assume(false);                                   \
                } while (0)
#else
        #include <cstdlib>
        #define UNREACHABLE(_message_if_reached)                           \
                do                                                         \
                {                                                          \
                        REPORT_UNREACHABLE_VIOLATION(_message_if_reached); \
                        std::abort();                                      \
                } while (0)
#endif

#endif // DEBUG_TOOLS_HPP
