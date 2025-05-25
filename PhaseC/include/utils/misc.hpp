#ifndef UTILS_MISC_HPP
#define UTILS_MISC_HPP

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include "debug_tools.hpp"
#include "utils/format_adapter.hpp"
#include "utils/smart_assert.h"

#if defined(__GNUC__) || defined(__clang__)
        #define ALWAYS_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
        #define ALWAYS_INLINE inline __forceinline
#else
        #define ALWAYS_INLINE inline
#endif
namespace Utils
{
[[nodiscard]] inline std::string str_to_lower(std::string str)
{
        std::transform(str.begin(), str.end(), str.begin(),
                       [](const unsigned char c) { return std::tolower(c); });
        return str;
}

template<typename N>
        requires std::is_integral_v<N>
[[nodiscard]] constexpr bool is_odd(N n) noexcept { return n % 2; }

template<typename N>
        requires std::is_integral_v<N>
[[nodiscard]] constexpr bool is_even(N n) noexcept { return !is_odd(n); }

inline char *cstrdup(const char *src)
{
        DEBUG_SMART_ASSERT(!!src);
        if(!src) [[unlikely]]
                return nullptr;

        const auto src_size = std::strlen(src) + 1; // +1 for NULL-byte
        char *dest = new char[src_size];
        std::memcpy(dest, src, src_size);
        return dest;
}
} // namespace Utils
#endif // UTILS_MISC_HPP
