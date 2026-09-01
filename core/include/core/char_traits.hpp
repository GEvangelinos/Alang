#ifndef ALPHA_CHAR_TRAITS_HPP
#define ALPHA_CHAR_TRAITS_HPP

#include <array>
#include "support/string_tools.hpp"

namespace alpha
{
namespace detail
{
    consteval bool is_id_body_char(const unsigned char c)
    {
        return support::is_digit(c) || support::is_alpha(c) || c == '_';
    };

    constexpr auto g_id_body_table = []() consteval
    {
        std::array<bool, 256> result{};
        for (unsigned short uc = 0; uc <= std::numeric_limits<unsigned char>::max(); ++uc)
            result[uc] = detail::is_id_body_char(uc);
        return result;
    }();

    template<unsigned char uc>
    struct report_id_body_table_mismatch;
    // NEVER DEFINE (Whole point of this, is it errors upon use)

    // Because Templates are instantiated before code is executed (even at compile time context)
    // the only way to find at which position g_id_body_table is off, is to use recursive template instantiation
    // --- Note this assertion is no longer needed... As I now initialize the table algorithmically  --- //
    // --- and not manually (filling bool table with 0s and 1s) but it's still a smart piece of code --- //
    // --- so I am keeping it, as it took me lots of tinkering to make the recursive assertion work. --- //
    template<unsigned short Index = 0>
    consteval bool assert_id_body_table_integrity()
    {
        // Check for false positives:
        if constexpr (Index > std::numeric_limits<unsigned char>::max())
            return true;
        else if constexpr (!detail::is_id_body_char(Index) && g_id_body_table[Index])
            // False Positives
            report_id_body_table_mismatch<Index>{};
        else if constexpr (detail::is_id_body_char(Index) && !g_id_body_table[Index])
            // False Negatives
            report_id_body_table_mismatch<Index>{};
        else
            return assert_id_body_table_integrity<Index + 1>();
    }

    static_assert(
        assert_id_body_table_integrity(),
        "ID Table mismatch! See compiler output for index."
    );
} // namespace detail


[[nodiscard]] constexpr bool is_id_body_char(const unsigned char c)
{
    return detail::g_id_body_table[c];
};
} // namespace alpha
#endif //ALPHA_CHAR_TRAITS_HPP
