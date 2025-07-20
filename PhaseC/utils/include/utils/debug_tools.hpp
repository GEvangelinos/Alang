#ifndef DEBUG_TOOLS_HPP
#define DEBUG_TOOLS_HPP
#include "utils/format_adapter.hpp"
#include "utils/smart_assert.h"

#ifndef TO_STRING
#define TO_STRING(x) #x
#endif

#define ATTACH_CONTEXT(message) \
        FMT::format("{}:{} -> {}(): {}", __FILENAME__, __LINE__, __func__, (message))
#define ATTACH_CONTEXT_CT(message)(__FILENAME__ __LINE__ message)

#define UNIMPLEMENTED(message_if_reached)                                                           \
        do                                                                        \
        {                                                                         \
                throw std::logic_error(ATTACH_CONTEXT("Control flow reached unimplemented code")); \
        } while (0)

#if defined(__GNUC__) || defined(__clang__)
        #define ALWAYS_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
        #define ALWAYS_INLINE inline __forceinline
#else
        #define ALWAYS_INLINE inline
#endif
#ifdef DEBUG_MODE
        #define DEBUG(...) __VA_ARGS__
        #define DEBUG_ALWAYS_INLINE // Disable inlining to ensure that a function visible debug symbols.
        #define DEBUG_NULLIFY(pointer) ((pointer) = nullptr)
        #define REQUIRE_PTR(ptr) Utils::require_ptr((ptr))
        #define DEBUG_REQUIRE_PTR(ptr) Utils::require_ptr((ptr))

//  Active in debug mode: evaluates and runs full SMART_ASSERT logic
        #define DEBUG_SMART_ASSERT(...) SMART_ASSERT(__VA_ARGS__)
//  Also active in debug mode: same as above, runs full logic
        #define DEBUG_SMART_ASSERT_EVAL(...) SMART_ASSERT(__VA_ARGS__)
#else
        #define DEBUG(...) // Ignore input!
        #define DEBUG_ALWAYS_INLINE ALWAYS_INLINE
        #define DEBUG_NULLIFY(pointer) ((void)0)
        #define REQUIRE_PTR(ptr) Utils::require_ptr_fast((ptr))
        #define DEBUG_REQUIRE_PTR(ptr) (ptr)

//  In release mode: disables assertion and also skips evaluating expressions (no side effects)
        #define DEBUG_SMART_ASSERT(...) ((void)0)
//  In release mode: disables assertion logic, but still evaluates expressions (preserves side effects)
        #define DEBUG_SMART_ASSERT_EVAL(...) ((void)(__VA_ARGS__))
#endif

#define REPORT_UNREACHABLE_VIOLATION(_message_if_reached)                  \
        do                                                                 \
        {                                                                  \
                std::cerr << ATTACH_CONTEXT(FMT::format(                   \
                        "Unreachable Violation: {}", _message_if_reached)) \
                << std::endl;                                              \
        } while (0)

#if defined(__GNUC__) || defined(__clang__)
        #define UNREACHABLE(_message_if_reached)                           \
                do                                                         \
                {                                                          \
                REPORT_UNREACHABLE_VIOLATION(_message_if_reached);         \
                __builtin_unreachable();                                   \
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
