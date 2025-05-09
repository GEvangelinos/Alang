#ifndef UTILS_MISC_HPP
#define UTILS_MISC_HPP

#include <algorithm>
#include <string>
#include "core/alpha_macros.hpp"

namespace // (Anonymous)
{
        inline std::string str_to_lower(std::string str)
        {
                std::transform(str.begin(), str.end(), str.begin(),
                               [](unsigned char c)
                               { return std::tolower(c); });
                return str;
        }

        template <typename N>
                requires std::is_integral_v<N>
        DEBUG_ALWAYS_INLINE constexpr bool is_odd(N n) noexcept
        {
                return n % 2;
        }

        template <typename N>
                requires std::is_integral_v<N>
        DEBUG_ALWAYS_INLINE constexpr bool is_even(N n) noexcept
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

        const T &get()
        {
                if (!assigned_)
                        throw std::logic_error("`Once` not assigned yet");
                return value_;
        }

        bool assigned() { return assigned_; }

private:
        T value_;
        bool assigned_ = false;
};

#endif // UTILS_MISC_HPP