#include "scanner/scanner_diagnostic_handlers.hpp"

#include "core/machine_types.hpp"
#include "diagnostics/diagnostic_reporter.gen.hpp"
#include "support/string_tools.hpp"

namespace alpha::scanner_diagnostic_handlers
{
AlphaInt
handle_invalid_integer_suffix(
    DiagnosticReporter &dr,
    const char *const text,
    const u64 text_len,
    const SourceLocation &full_literal_loc)
{
    DMASSERT(!!text, !!text_len);

    const char *start = text;
    const char *const end = text + text_len;

    int base = 10;

    if (start[0] == '0' && (start[1] == 'x' || start[1] == 'X'))
    {
        base = 16;
        start += 2;
        while (support::is_xdigit(*start))
            ++start;
    }
    else
    {
        DMASSERT(support::is_digit(start[0]));
        ++start;
        while (support::is_digit(*start))
            ++start;
    }

    DMASSERT(start < end);

    const u64 suffix_offset = static_cast<u64>(start - text);
    const std::string integer{text, text_len - suffix_offset};
    const std::string_view suffix{start, text_len - suffix_offset};
    const SourceLocation suffix_loc{
        SrcBuffIdx{
            static_cast<SrcBuffIdx::UnderlyingType>(
                full_literal_loc.begin.value + (text_len - suffix.size()))
        },
        full_literal_loc.end
    };
    dr.report_invalid_numeric_suffix("integer", suffix, full_literal_loc, suffix_loc);

    return std::stoll(integer, nullptr, base);
}
} // namespace alpha::scanner_diagnostic_adapters
