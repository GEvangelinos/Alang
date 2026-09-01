#include "scanner/scanner_diagnostic_handlers.hpp"

#include <charconv>

#include "core/machine_types.hpp"
#include "core/char_traits.hpp"
#include "diagnostics/diagnostic_reporter.gen.hpp"
#include "support/string_tools.hpp"

namespace
{
using namespace alpha;

template<typename T>
concept NumericType = std::integral<T> || std::floating_point<T>;


template<NumericType T>
void
make_invalid_numeric_suffix_report(
    DiagnosticReporter &dr,
    const char *const cursor,
    const std::string_view lexeme,
    const SourceLocation lexeme_loc,
    const char *const numeric_type_name)
{
    DMASSERT(
        cursor > lexeme.begin() &&
        cursor < lexeme.end() &&
        "cursor must be strictly within the lexeme bounds"
    );
    const u64 number_length = static_cast<u64>(cursor - lexeme.begin());
    const std::string_view suffix{cursor, lexeme.end()};
    const SourceLocation suffix_loc{lexeme_loc.begin + number_length, lexeme_loc.end};
    dr.report_invalid_numeric_suffix(numeric_type_name, suffix, lexeme_loc, suffix_loc);
}
} // namespace

namespace alpha::scanner_diagnostic_handlers
{
AlphaInt
handle_invalid_integer_suffix(
    DiagnosticReporter &dr,
    const std::string_view lexeme,
    const SourceLocation lexeme_loc)
{
    const char *scan_start = lexeme.begin();
    int base = 10;

    if (lexeme[0] == '0' && (lexeme[1] == 'x' || lexeme[1] == 'X'))
    {
        base = 16;
        scan_start += 2;
    }

    AlphaInt integer_value = 0;
    const auto [cursor, ec] = std::from_chars(scan_start, lexeme.end(), integer_value, base);

    if (cursor[0] == 'e' || cursor[0] == 'E' )
    {
        if (is_id_body_char() cursor[0])
    }

    make_invalid_numeric_suffix_report<AlphaInt>(dr, cursor, lexeme, lexeme_loc, "integer");
    return integer_value;
}

AlphaFloat
handle_invalid_float_suffix(
    DiagnosticReporter &dr,
    const std::string_view lexeme,
    const SourceLocation lexeme_loc)
{
    AlphaFloat float_value = 0.0;
    const auto [cursor, ec] = std::from_chars(lexeme.begin(), lexeme.end(), float_value);
    make_invalid_numeric_suffix_report<AlphaFloat>(dr, cursor, lexeme, lexeme_loc, "float");
    return float_value;
}
} // namespace alpha::scanner_diagnostic_handlers
