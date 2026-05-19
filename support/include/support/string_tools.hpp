#ifndef SUPPORT_STRING_TOOLS_HPP
#define SUPPORT_STRING_TOOLS_HPP

#include <cstring>
#include <string>
#include <vector>

#include "support/debug_tools.hpp"

namespace alpha::support
{
[[nodiscard]] std::string tolower_str(std::string str);
[[nodiscard]] std::string toupper_str(std::string str);
std::string& lstrip(std::string& str);
std::string& rstrip(std::string& str);
std::string& strip(std::string& str);
[[nodiscard]] bool is_blank_str(const std::string& str);
[[nodiscard]] std::vector<std::string> split_lines(const std::string& str);

template <typename T, unsigned char precision = 6>
    requires std::is_floating_point_v<T>
[[nodiscard]] std::string format_float(const T value)
{
    std::string s = FMT::format("{:.{}f}", value, precision);
    if constexpr (precision > 0)
    {
        s.erase(s.find_last_not_of('0')+ 1);
        if (s.back() == '.')
            s.pop_back();
    }
    return s + "f";
}

// Basic implementation, so we can avoid using the slower std::isalpha() that looks up for locale.
[[nodiscard]] constexpr bool is_alpha(const unsigned char c) noexcept
{
    // Fold uppercase into lowercase by setting bit 5
    constexpr unsigned char mask = 0b0010'0000;
    const unsigned char lower = c | mask; // Destructive operation. Only works for a-zA-Z .

    DEBUG(
        if (!std::is_constant_evaluated())
        if (std::isalpha(c))
        DMASSERT(lower == static_cast<decltype(c)>(std::tolower(c)));
    )

    const bool result = lower >= 'a' && lower <= 'z';
    if (!std::is_constant_evaluated()) // std::isalpha() is not constexpr at the time of writing.
        DMASSERT(result == !!std::isalpha(c));
    return result;
}

// Basic implementation, so we can avoid using the slower std::isalpha() that looks up for locale.
[[nodiscard]] constexpr bool is_space(const unsigned char c) noexcept
{
    switch (c)
    {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v': return true;
    default: return false;
    }
}

static_assert(
    []()
    {
        for (unsigned int c = 0; c <= static_cast<unsigned char>(-1); ++c)
        {
            const bool expected = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
            if (support::is_alpha(c) != expected)
                return false;
        }
        return true;
    }(),
    "This is a Compile Time Proof, DO NOT REMOVE. If this static assert fails, "
    "it means the mask trick support::is_alpha() is using, is wrong, and produces wrong results."
);

[[nodiscard]] inline char* cstrdup(const char* const src, const std::size_t size)
{
    DMASSERT(!!src);
    if (!src)
        return nullptr;

    // +1 for the null terminator is mandatory for "cstr" functions
    char* const dest = new(std::nothrow) char[size + 1];
    if (!dest)
        return nullptr;
    std::memcpy(dest, src, size);
    return dest;
}

// Basic implementation, so we can avoid using the slower std::isdigit() that looks up for locale.
[[nodiscard]] constexpr bool is_digit(const unsigned char c) noexcept
{
    const bool result = c >= '0' && c <= '9';
    if (!std::is_constant_evaluated()) // std::isdigit() is not constexpr at the time of writing.
        DMASSERT(result == !!std::isdigit(c));
    return result;
}

// Basic implementation, so we can avoid using the slower std::isxdigit() that looks up for locale.
[[nodiscard]] constexpr bool is_xdigit(const unsigned char c) noexcept
{
    // Fold uppercase into lowercase by setting bit 5
    constexpr unsigned char mask = 0b0010'0000;
    const unsigned char lower = c | mask; // Destructive operation. Only works for 0-9a-zA-Z .

    DEBUG(
        if (!std::is_constant_evaluated())
        if (std::isalpha(c))
        DMASSERT(lower == static_cast<decltype(c)>(std::tolower(c)));
    )

    const bool result = (c >= '0' && c <= '9') || (lower >= 'a' && lower <= 'f');
    if (!std::is_constant_evaluated()) // std::isxdigit() is not constexpr at the time of writing.
        DMASSERT(result == !!std::isxdigit(c));
    return result;
}
} // namespace alpha::support
#endif // SUPPORT_STRING_TOOLS_HPP
