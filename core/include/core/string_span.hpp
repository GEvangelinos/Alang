/**
 * StringSpan: A Trivial POD for compiler internals.
 * Unlike std::string_view, this struct is guaranteed to be "Trivial" and a
 * "Standard Layout" type, making it safe for use in 'union' structures
 * (Tokens/AST) without requiring custom constructors. It avoids
 * implementation-defined overhead, enables compile-time literal sizing
 * via 'consteval', and ensures binary predictability across ABIs.
 */

#ifndef STRING_SPAN_HPP
#define STRING_SPAN_HPP

#include <cstring>
#include <type_traits>
#include <string>
#include "numeric_types.hpp"

namespace alpha
{
struct StringSpan
{
    const char* data;
    u64 size;

    [[nodiscard]] bool empty() const { return size == 0; }
    [[nodiscard]] const char* begin() const { return data; }
    [[nodiscard]] const char* end() const { return data + size; }
    [[nodiscard]] std::string to_string() const { return {data, size}; }
    [[nodiscard]] std::string_view to_string_view() const noexcept { return {data, size}; }

    [[nodiscard]] constexpr bool operator==(const StringSpan other) const noexcept
    {
        return std::string_view{data, size} == std::string_view{other.data, other.size};
    }

    void clear() noexcept
    {
        data = nullptr;
        size = 0;
    }

    template <u64 size>
    [[nodiscard]] static consteval StringSpan from_literal(const char (&str)[size]) noexcept
    {
        // Subtracting 1 is mandatory to exclude the null terminator.
        return StringSpan{.data = str, .size = size - 1};
    }

    [[nodiscard]] static constexpr StringSpan from_string(const std::string& str) noexcept
    {
        return StringSpan{.data = str.data(), .size = str.size()};
    }

    [[nodiscard]] static StringSpan from_cstring(const char* const str) noexcept
    {
        return StringSpan{.data = str, .size = std::strlen(str)};
    }
};

[[nodiscard]] char* duplicate_to_cstring(StringSpan ss);

static_assert(StringSpan::from_literal("").size == 0);
static_assert(std::is_trivial_v<StringSpan>);
} // namespace alpha

template <>
struct std::hash<alpha::StringSpan>
{
    [[nodiscard]] std::size_t operator()(const alpha::StringSpan ss) const noexcept
    {
        return std::hash<std::string_view>{}(std::string_view{ss.data, ss.size});
    }
};

#endif // STRING_SPAN_HPP
