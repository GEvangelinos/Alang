#ifndef ALPHA_MACROS_HPP
#define ALPHA_MACROS_HPP

#include "utils/format_adapter.hpp"
#include <iostream>

#define REPORT_UNREACHABLE_VIOLATION(_message_if_reached)                                   \
        do                                                                                  \
        {                                                                                   \
                std::cerr << FMT::format("{}:{} | {}() --> Unreachable Violation: {}",      \
                                         __FILE__, __LINE__, __func__, _message_if_reached) \
                          << std::endl;                                                     \
        } while (0)

// clang-format off
#ifdef DEBUG_MODE
        // Disable inlining to ensure that a function visible debug symbols.
        #define DEBUG_ALWAYS_INLINE
#elif defined(__GNUC__) || defined(__clang__)
        #define DEBUG_ALWAYS_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
        #define DEBUG_ALWAYS_INLINE inline __forceinline
#else
        #define DEBUG_ALWAYS_INLINE inline
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
// clang-format on

#endif // ALPHA_MACROS_HPP
