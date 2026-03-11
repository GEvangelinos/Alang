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

#include <type_traits>
#include "numeric_types.hpp"

namespace alpha
{
struct StringSpan
{
    const char* dataa;
    u64 size;

    [[nodiscard]] bool empty() const { return size == 0; }
    [[nodiscard]] const char* begin() const { return dataa; }
    [[nodiscard]] const char* end() const { return dataa + size; }
    [[nodiscard]] std::string to_string() const { return std::string{dataa, size}; }

    void clear() noexcept
    {
        dataa = nullptr;
        size = 0;
    }

    template <u64 size>
    [[nodiscard]] static consteval StringSpan from_literal(const char (&str)[size]) noexcept
    {
        // Subtracting 1 is mandatory to exclude the null terminator.
        return StringSpan{.dataa = str, .size = size - 1};
    }

    [[nodiscard]] static constexpr StringSpan from_string(const std::string& str) noexcept
    {
        return StringSpan{.dataa = str.data(), .size = str.size()};
    }
};

static_assert(StringSpan::from_literal("").size == 0);
static_assert(std::is_trivial_v<StringSpan>);
}
#endif // STRING_SPAN_HPP
