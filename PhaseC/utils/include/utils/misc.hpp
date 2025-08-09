#ifndef UTILS_MISC_HPP
#define UTILS_MISC_HPP

#include <algorithm>
#include <cstring>
#include <iostream>
#include <source_location>
#include <string>
#include "utils/debug_tools.hpp"
#include "utils/format_adapter.hpp"
#include "utils/smart_assert.h"

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
    if (!src)
        return nullptr;

    const auto src_size = std::strlen(src) + 1; // +1 for NULL-byte
    char *dest = new char[src_size];
    std::memcpy(dest, src, src_size);
    return dest;
}

// Utility function used to mainly assert pointers
// in initialization lists of constructors.
template<typename T>
[[nodiscard]] T *require_ptr(T *const ptr,
                             const std::source_location loc = std::source_location::current())
{
    if (ptr) [[likely]]
            return ptr;
    std::cerr << FMT::format(
                "ERROR: nullptr caught (expected valid pointer)\n"
                "{}:{}:{} -> {}: ",
                loc.file_name(),
                loc.line(),
                loc.column(),
                loc.function_name())
            << std::endl;
    std::abort();
}

// Just like `require_ptr`, but this one is skinny. Good for inlining, required in hot paths.
template<typename T>
[[nodiscard]] ALWAYS_INLINE T *require_ptr_fast(T *const ptr)
{
    if (ptr) [[likely]]
            return ptr;
    std::abort();
}

template<typename FloatType, typename IntType>
    requires std::is_floating_point_v<FloatType> && std::is_integral_v<IntType>
bool is_lossless_int_to_float(IntType i)
{
    return i == static_cast<IntType>(static_cast<FloatType>(i));
}
} // namespace Utils

#endif // UTILS_MISC_HPP
