#ifndef SUPPORT_MISC_TOOLS_HPP
#define SUPPORT_MISC_TOOLS_HPP

#include <iostream>
#include <source_location>
#include <string>
#include "support/format_adapter.hpp"

namespace alpha::support
{
template<typename N>
    requires std::is_integral_v<N>
[[nodiscard]] constexpr bool is_odd(N n) noexcept { return n % 2; }

template<typename N>
    requires std::is_integral_v<N>
[[nodiscard]] constexpr bool is_even(N n) noexcept { return !is_odd(n); }

// Utility function used to mainly assert pointers
// in initialization lists of constructors.
template<typename T>
[[nodiscard]] T *require_ptr(
    T *const ptr, const std::source_location loc = std::source_location::current())
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

#ifdef DEBUG_MODE
#define DEBUG_REQUIRE_PTR(ptr) alpha::support::require_ptr(ptr)
#else
#define DEBUG_REQUIRE_PTR(ptr) (ptr)
#endif

template<typename FloatType, typename IntType>
    requires std::is_floating_point_v<FloatType> && std::is_integral_v<IntType>
bool is_lossless_int_to_float(IntType i)
{
    return i == static_cast<IntType>(static_cast<FloatType>(i));
}
} // namespace alpha::support

#endif // SUPPORT_MISC_TOOLS_HPP
