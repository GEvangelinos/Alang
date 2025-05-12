#ifndef UTILS_MISC_HPP
#define UTILS_MISC_HPP

#include <algorithm>
#include <string>
#include <iostream>
#include "utils/format_adapter.hpp"
#include "utils/smart_assert.h"

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

namespace // (Anonymous)
{
        [[nodiscard]] inline std::string str_to_lower(std::string str)
        {
                std::transform(str.begin(), str.end(), str.begin(),
                               [](unsigned char c)
                               { return std::tolower(c); });
                return str;
        }

        template <typename N>
                requires std::is_integral_v<N>
        [[nodiscard]] constexpr bool is_odd(N n) noexcept
        {
                return n % 2;
        }

        template <typename N>
                requires std::is_integral_v<N>
        [[nodiscard]] constexpr bool is_even(N n) noexcept
        {
                return !is_odd(n);
        }
} // namespace

/// @brief A mixin that prevents copying, moving, or reassigning derived classes.
/// @details
///   Inherit from this class to make your type “immobile”:
///   once constructed, instances cannot be copied, moved, or reassigned.
///   Note that this does *not* freeze internal state—mutable members
///   or methods marked `const` may still modify their own data.
///
/// @note
///   Copy-construction, move-construction, copy-assignment, and move-assignment
///   are all deleted. Derived types remain fully mutable through their own
///   member functions.
class Immobile
{
public:
        Immobile() = default;
        ~Immobile() = default;

        Immobile(const Immobile &) = delete;            ///< Disable copy-construction
        Immobile(Immobile &&) = delete;                 ///< Disable move-construction
        Immobile &operator=(const Immobile &) = delete; ///< Disable copy-assignment
        Immobile &operator=(Immobile &&) = delete;      ///< Disable move-assignment
};

class ToggleSwitch
{
public:
        void enable() noexcept
        {
                DEBUG_SMART_ASSERT(is_disabled());
                state_ = true;
        }
        void disable() noexcept
        {
                DEBUG_SMART_ASSERT(is_enabled());
                state_ = false;
        }
        [[nodiscard]] bool is_enabled() const noexcept { return state_; }
        [[nodiscard]] bool is_disabled() const noexcept { return !state_; }

private:
        bool state_ = false; // Initially the switch is off.
};

template <typename T>
class Once : private Immobile
{
public:
        Once() = default;
        ~Once() = default;

        void set(T value)
        {
                if (assigned_)
                        throw std::logic_error("`Once` already assigned");
                value_ = value;
                assigned_ = true;
        }

        [[nodiscard]] const T &get()
        {
                if (!assigned_)
                        throw std::logic_error("`Once` not assigned yet");
                return value_;
        }

        [[nodiscard]] bool assigned() { return assigned_; }

private:
        T value_;
        bool assigned_ = false;
};

#endif // UTILS_MISC_HPP