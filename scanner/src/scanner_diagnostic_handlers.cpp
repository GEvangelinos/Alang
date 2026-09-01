#include "scanner/scanner_diagnostic_handlers.hpp"

#include <charconv>

#include "core/machine_types.hpp"
#include "core/char_traits.hpp"
#include "diagnostics/diagnostic_reporter.gen.hpp"
#include "support/string_tools.hpp"

namespace
{
using namespace alpha;

[[nodiscard]] SourceLocation
extract_suffix_location(
    const char *const cursor,
    const std::string_view lexeme,
    const SourceLocation lexeme_loc
)
{
    DMASSERT(
        cursor > lexeme.begin() &&
        "Numeric prefix underflow: cursor must advance past at least one valid "
        "leading digit/prefix character (cursor == lexeme.data()).",
        cursor < lexeme.end() &&
        "Empty suffix / Lexer routing bug: from_chars consumed the entire lexeme. "
        "A valid numeric literal was mistakenly routed to an invalid suffix diagnostic handler."
    );

    const u64 number_length = static_cast<u64>(cursor - lexeme.begin());
    const SourceLocation suffix_loc{lexeme_loc.begin + number_length, lexeme_loc.end};
    return suffix_loc;
}

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
    const std::string_view suffix{cursor, lexeme.end()};
    const SourceLocation suffix_loc = extract_suffix_location(cursor, lexeme, lexeme_loc);
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
    int base = 10;
    const char *scan_start = lexeme.begin();


    const bool is_hex =
            lexeme.size() >= 3 &&
            lexeme[0] == '0' &&
            (lexeme[1] == 'x' || lexeme[1] == 'X') &&
            support::is_xdigit(lexeme[2]);
    if (is_hex)
    {
        base = 16;
        scan_start += 2;
    }

    AlphaInt integer_value = 0;
    const auto [cursor, ec] = std::from_chars(scan_start, lexeme.end(), integer_value, base);
    make_invalid_numeric_suffix_report(dr, cursor, lexeme, lexeme_loc, "integer");
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
    make_invalid_numeric_suffix_report(dr, cursor, lexeme, lexeme_loc, "float");
    return float_value;
}

AlphaFloat
handle_missing_exponent_digits(
    DiagnosticReporter &dr,
    const std::string_view lexeme,
    const SourceLocation lexeme_loc)
{
    const char *scan_start = lexeme.begin();
    AlphaFloat float_value = 0.0;
    const auto [cursor, ec] = std::from_chars(scan_start, lexeme.end(), float_value);
    DMASSERT(cursor < lexeme.end() , *cursor == 'e' || *cursor == 'E');

    const u64 number_length = static_cast<u64>(cursor - lexeme.begin());
    const SrcBuffIdx exponent_src_buff_idx = lexeme_loc.begin + number_length;
    const SourceLocation exponent_loc{exponent_src_buff_idx, exponent_src_buff_idx + 1};

    dr.report_missing_exponent_digits(lexeme_loc, exponent_loc);
    return float_value;
}
} // namespace alpha::scanner_diagnostic_handlers
