#include "diagnostics/diagnostic_formatter.hpp"

#include <cctype>
#include <string>

#include "core/source_location.hpp"
#include "diagnostics/diagnostic_types.hpp"
#include "diagnostics/issue_formatter.hpp"
#include "support/misc_tools.hpp"
#include "support/string_tools.hpp"

namespace alpha
{
DiagnosticFormatter::DiagnosticFormatter(
    const std::filesystem::path &source_path,
    const LocationTracker &loc_tracker,
    const TranslationUnitBuffer &source_buffer,
    const bool colorize)
    : issue_formatter_(std::make_unique<IssueFormatter>(
        source_path, source_buffer, loc_tracker, colorize
    )) {}

DiagnosticFormatter::~DiagnosticFormatter()
{
    issue_formatter_.reset();
}

std::string
DiagnosticFormatter::format(const Diagnostic &diagnostic) const
{
    DMASSERT(!!issue_formatter_);

    std::stringstream ss;
    ss << issue_formatter_->format(diagnostic.primary);
    for (const Note &note : diagnostic.note_list)
        ss << issue_formatter_->format(note);
    return ss.str();
}
} // namespace alpha

namespace
{
[[maybe_unused, deprecated("Used in old diagnostic system")]]
std::string expand_tabs(const std::string_view line, const int tab_width = 8)
{
    std::string result;
    result.reserve(line.size() + std::count(line.begin(), line.end(), '\t') * (tab_width - 1));

    alpha::uf64 col = 0;
    for (const char ch : line)
    {
        if (ch == '\t')
        {
            const int spaces = tab_width - col % tab_width;
            result.append(spaces, ' ');
            col += spaces;
        }
        else
        {
            result += ch;
            ++col;
        }
    }
    return result;
}

[[maybe_unused, deprecated("Used in old diagnostic system")]]
std::string capture_next_line(const char *const buffer)
{
    std::string line;
    for (auto idx = 0; buffer[idx] != '\n' && buffer[idx] != '\0'; ++idx)
        line.push_back(buffer[idx]);
    return line;
}

// Compute the visual caret offset for a given line.
// Tabs are tricky because their displayed width depends on the current column.
// For each tab, we advance to the next multiple of `tab_width` columns.
[[maybe_unused, deprecated("Used in old diagnostic system")]]
int compute_visual_caret_offset(
    const std::string_view line,
    const alpha::uf64 raw_offset,
    const int tab_width = 8)
{
    alpha::uf64 col = 0;
    for (alpha::uf64 i = 0; i < raw_offset; ++i)
        col += line[i] == '\t' ? tab_width - col % tab_width : 1;
    return col;
}
} // namespace
