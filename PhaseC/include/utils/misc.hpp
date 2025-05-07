#ifndef UTILS_MISC_HPP
#define UTILS_MISC_HPP

#include <algorithm>
#include <string>
namespace // (Anonymous)
{

inline std::string str_to_lower(std::string str)
{
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) { return std::tolower(c); });
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
        !is_odd(n);
}
} // namespace

#endif // UTILS_MISC_HPP