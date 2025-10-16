#include "diagnostics/diagnostic_formatter.hpp"

#include <cctype>
#include <string>

#include "../../arguinator/include/arguinator/arguinator.hpp"
#include "core/source_location.hpp"
#include "diagnostics/diagnostic_types.hpp"
#include "support/misc_tools.hpp"
#include "support/string_tools.hpp"
#include "diagnostics/issue_formatter.hpp"

namespace alpha
{
DiagnosticFormatter::DiagnosticFormatter(
    const std::filesystem::path &source_path,
    const LocationTracker &loc_tracker,
    const char *const source_buffer,
    const bool colorize)
    : issue_formatter_(std::make_unique<IssueFormatter>(
        source_path, source_buffer, loc_tracker, colorize
    )) {}

DiagnosticFormatter::~DiagnosticFormatter()
{
    issue_formatter_.reset();
}

// void
// DiagnosticFormatter::format_issue_line(
//     std::stringstream &out,
//     const Issue &issue,
//     const SrcLineIdx line_no,
//     const bool colorize) const
// {
//     DEBUG_SMART_ASSERT(line_no.value > 0 && "Line number is invalid (lines start at 1).");
//
//     const char *const suggestion_color = colorize ? COLOR_FG_ASCII_GREEN : "";
//     const char *const underline_color = colorize ? get_underline_color(issue.type) : "";
//     const char *const reset_sgr = colorize ? SGR_RESET : "";
//
//     std::string codeline = make_codeline(issue, line_no);
//     codeline = colorize_line_comment(codeline);
//     std::string underline = make_underline(issue, line_no);
//     swap_markers(underline, Markers::highlight, Markers::underline);
//     if (support::is_blank_str(codeline) &&
//         support::is_blank_str(underline) &&
//         !issue.suggestion.has_value())
//         return;
//
//     SrcLineIdx suggestion_line_no{SrcLineIdx::none};
//     if (issue.suggestion.has_value())
//         suggestion_line_no = loc_tracker_.find_last_line(issue.suggestion->insert_after);
//     if (!issue.suggestion.has_value() || suggestion_line_no != line_no)
//     {
//         out << FMT::format("{0:>{1}} | {2}\n", line_no.value, k_linebox_width_, codeline);
//         if (!underline.empty())
//             out << FMT::format(
//                 "{0:>{1}} | {2}\n",
//                 "",                                              //{0}
//                 k_linebox_width_,                                //{1}
//                 apply_sgr(underline_color, underline, reset_sgr) // {2}
//             );
//     }
//     else
//     {
//         DEBUG_SMART_ASSERT(issue.suggestion.has_value());
//         const u32 split_point = compute_visual_suggestion_indent_width(issue.suggestion.value());
//
//         out << FMT::format(
//             "{0:>{1}} | {2}{3}{4}\n",
//             line_no.value,                                //{0}
//             k_linebox_width_,                             //{1}
//             codeline.substr(0, split_point),              //{2}
//             apply_sgr(suggestion_color, "  ", reset_sgr), //{2}
//             codeline.substr(split_point)                  //{4}
//         );
//
//         if (!underline.empty())
//             out << FMT::format(
//                 "{0:>{1}} | {2}\n",
//                 "",               //{0}
//                 k_linebox_width_, //{1}
//                 apply_sgr(suggestion_color, std::string(split_point, ' ') + " " + "^",
//                           reset_sgr) // {2}
//             );
//
//         if (!underline.empty())
//             out << FMT::format(
//                 "{0:>{1}} | {2}\n",
//                 "",                                                                          //{0}
//                 k_linebox_width_,                                                            //{1}
//                 apply_sgr(suggestion_color, std::string(split_point, ' ') + " |", reset_sgr) // {2}
//             );
//         if (!underline.empty())
//             out << FMT::format(
//                 "{0:>{1}} | {2}\n",
//                 "",               //{0}
//                 k_linebox_width_, //{1}
//                 apply_sgr(suggestion_color,
//                           std::string(split_point, ' ') + " " + issue.suggestion->desc,
//                           reset_sgr) // {2}
//             );
//     }
//
//     const std::vector<std::string> highlight_roots = make_highlight_root_stems(
//         issue, line_no, 1, Markers::highlight_stem_root);
//     const std::vector<std::string> highlight_roots2;
//     const std::vector<std::string> highlight_stems = make_highlight_stems(issue, line_no);
//     std::vector<std::string> label_lines;
//     label_lines.reserve(highlight_roots.size() * 2 + highlight_stems.size());
//     label_lines.insert(
//         label_lines.end(),
//         std::make_move_iterator(highlight_roots.begin()),
//         std::make_move_iterator(highlight_roots.end())
//     );
//     label_lines.insert(
//         label_lines.end(),
//         std::make_move_iterator(highlight_roots2.begin()),
//         std::make_move_iterator(highlight_roots2.end())
//     );
//     label_lines.insert(
//         label_lines.end(),
//         std::make_move_iterator(highlight_stems.begin()),
//         std::make_move_iterator(highlight_stems.end())
//     );
//     std::string label_box;
//     for (const auto &line : label_lines)
//     {
//         if (support::is_blank_str(line))
//             continue;
//
//         label_box += FMT::format(
//             "{0:>{1}} | {2}\n",
//             "",               //{0}
//             k_linebox_width_, //{1}
//             line              //{2}
//         );
//     }
//     out << label_box;
//     out << reset_sgr;
// }

std::string
DiagnosticFormatter::format(const Diagnostic &diagnostic) const
{
    DEBUG_SMART_ASSERT(!!issue_formatter_);

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
