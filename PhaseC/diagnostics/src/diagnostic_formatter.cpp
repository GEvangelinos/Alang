#include "diagnostics/diagnostic_printer.hpp"
#include <string>
#include <cctype>

#include "core/source_location.hpp"
#include "diagnostics/diagnostic_types.hpp"
#include "utils/cli_color.h"
#include "utils/misc.hpp"

namespace
{
[[nodiscard]] std::string assemble_next_line(const char *buffer)
{
    std::string line;
    for (std::size_t i = 0; buffer[i] != '\n' && buffer[i] != '\0'; ++i)
        line += buffer[i];

    return std::move(line);
}

std::vector<std::string_view> extract_line_views(
    const char *const buffer,
    const std::size_t start_index,
    const std::size_t end_index)
{
    std::vector<std::string_view> lines;
    const char *start = buffer + start_index;
    const char *current = start;
    const char *end_target = buffer + end_index;

    while (*current)
    {
        const char *line_end = std::strchr(current, '\n');
        if (!line_end)
            line_end = current + std::strlen(current); // to '\0' if no newline

        lines.emplace_back(current, line_end - current); // view [current, line_end)

        if (end_target <= line_end)
            break; // stop if end_index is in this line

        current = *line_end == '\n' ? line_end + 1 : line_end;
    }
    return lines;
}

std::string expand_tabs(const std::string_view line, const int tab_width = 8)
{
    std::string result;
    result.reserve(line.size() + std::count(line.begin(), line.end(), '\t') * (tab_width - 1));

    alpha::uf64 col = 0;
    for (const char ch: line)
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

// Compute the visual caret offset for a given line.
//
// Tabs are tricky because their displayed width depends on the current column.
// For each tab, we advance to the next multiple of `tab_width` columns.
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

namespace alpha
{
DiagnosticFormatter::DiagnosticFormatter(
    const std::filesystem::path &source_path,
    const LocationTracker &loc_tracker,
    const char *const source_buffer)
    : source_filename_(source_path.string()),
      source_buffer_(utils::require_ptr(source_buffer)),
      loc_tracker_(loc_tracker) {}

const char *
DiagnosticFormatter::highlight_color(const Issue::Type type) noexcept
{
    using IT = Issue::Type;
    switch (type)
    {
    case IT::NOTE: return COLOR_ASCII_BOLD_CYAN;
    case IT::WARNING: return COLOR_ASCII_BOLD_MAGENTA;
    case IT::SOFT_ERROR:
    case IT::HARD_ERROR:
    case IT::FATAL_ERROR: return COLOR_ASCII_BOLD_RED;
    default: UNREACHABLE("Unknown Issue Type!");
    }
}

std::string
DiagnosticFormatter::build_issue_header(const Issue &issue, const bool colorize) const
{
    const u32 line = issue.line(loc_tracker_);
    const u32 column = issue.column(loc_tracker_);

    const char *const header_sgr = colorize ? COLOR_ASCII_BOLD_DEFAULT : "";
    const char *const type_color = colorize ? highlight_color(issue.type) : "";
    const char *const reset_sgr = colorize ? SGR_RESET : "";

    std::stringstream ss;
    ss << header_sgr;

    ss << FMT::format(
        "{0}:{1}:{2}: {3}{4}{5}: {6}",
        source_filename_,      // {0} file
        line,                  // {1} line
        column,                // {2} col
        type_color,            // {3} color for issue type
        to_string(issue.type), // {4} issue type text
        header_sgr,            // {5} restore header style
        issue.desc             // {6} description
    );

    ss << reset_sgr;
    return ss.str();
}

std::string
DiagnosticFormatter::build_underline(const Issue &issue, const u32 line_no)
{
    std::string underline;

    // Get the buffer index at which this line starts
    const u32 line_start = loc_tracker_.find_index_of_line(line_no);

    // Walk characters until newline, building a highlight string
    for (auto idx = line_start; source_buffer_[idx] != '\n'; ++idx)
    {
        const bool outside_issue = idx < issue.loc.first_index || idx >= issue.loc.last_index;
        const char ch = source_buffer_[idx];
        underline += outside_issue || std::isspace(static_cast<unsigned char>(ch))
                     ? ' '
                     : '~';
    }
    return underline;
}

std::vector<std::string>
DiagnosticFormatter::build_suggestion_lines(const Suggestion &suggestion)
{
    const u32 line_no = loc_tracker_.find_last_line(suggestion.insert_after);
    const u32 line_start = loc_tracker_.find_index_of_line(line_no);

    DEBUG_SMART_ASSERT(suggestion.insert_after.last_index >= line_start);

    // How far into the line the suggestion should be indented
    const u32 indent_width = suggestion.insert_after.last_index - line_start + 1;
    const std::string indent(indent_width, ' ');

    std::vector<std::string> suggestion_lines;
    std::string current_line = indent;
    for (const char ch: suggestion.text)
    {
        if (ch == '\n')
        {
            suggestion_lines.push_back(current_line);
            current_line = indent; // clear and reset line to just space offset.
        }
        else
            current_line += ch;
    }

    // Push the last suggestion line (after the final '\n', or the whole text if no newline)
    suggestion_lines.push_back(current_line);

    return suggestion_lines;
}

std::string
DiagnosticFormatter::format_issue(const Issue &issue, const bool colorize) const
{
    std::stringstream ss;
    ss << build_issue_header(issue, colorize) << '\n';

    const Issue::RenderingSpan span = issue.compute_printing_span(loc_tracker_);
    u32 suggestion_line_no = 0;
    if (issue.suggestion.has_value())
        suggestion_line_no = loc_tracker_.find_last_line(issue.suggestion->insert_after);

    for (u32 line_no = span.start_line; line_no <= span.end_line; ++line_no)
    {
        DEBUG_SMART_ASSERT(line_no > 0 && "Line number is invalid (lines start at 1).");
        const auto starting_index = loc_tracker_.find_index_of_line(line_no);
        auto line = assemble_next_line(source_buffer_ + starting_index);
        std::cout << line;
        auto underline = build_underline(issue, line_no);
        std::cout << underline;
        if (suggestion_line_no == line_no)
        {
            DEBUG_SMART_ASSERT(issue.suggestion.has_value());
            std::vector<std::string> suggestion_lines =
                build_suggestion_lines(issue.suggestion.value());
            for (const auto &sline : suggestion_lines)
                std::cout << sline;
        }
    }
}

std::string
DiagnosticFormatter::format_diagnostic(const Diagnostic &diagnostic, const bool colorize) const
{
    std::stringstream ss;
    ss << format_issue(diagnostic.primary, colorize);
    for (const Note &note: diagnostic.note_list)
        ss << format_issue(note, colorize);
    return ss.str();
}
} // namespace alpha

// // TODO: Fix.. its ugly AF
// std::string
// Diagnostic::make_pretty_diagnostic_impl2(
//     const std::string &source_filename,
//     const LocationTracker &loc_tracker,
//     const char *input_buffer,
//     const Issue &issue)
// {
//     const u32 issue_line = issue.line(loc_tracker);
//     const u32 issue_column = issue.column(loc_tracker);
//     /* Error header: */
//     std::stringstream ss;
//     ss << COLOR_ASCII_BOLD_DEFAULT
//             << FMT::format("{}:{}:{}: {}{}{}: {}\n", source_filename, issue_line, issue_column,
//                            issue.pretty_color(), issue.type_to_string(), COLOR_ASCII_BOLD_DEFAULT,
//                            issue.desc)
//             << SGR_RESET;
//
//     const auto line_views = extract_line_views(
//         input_buffer, loc_tracker.find_index_of_line(issue_line), issue.loc.last_index);
//     for (std::size_t i = 0; i < line_views.size(); i++)
//     {
//         constexpr u32 line_box_width = 8;
//         std::string visual_line = expand_tabs(line_views[i]);
//         ss << FMT::format("{:>{}} | {}\n",
//                           i != 0 ? "" : std::to_string(issue_line), line_box_width, visual_line);
//         if (i != 0) // Caret marking is only for first line.
//             continue;
//
//         DEBUG_SMART_ASSERT(issue.loc.last_index > issue.loc.first_index);
//
//         const auto raw_caret_offset =
//                 issue.loc.first_index - loc_tracker.find_index_of_line(
//                     loc_tracker.find_first_line(issue.loc));
//         const auto visual_caret_offset =
//                 compute_visual_caret_offset(line_views[i], raw_caret_offset);
//         const auto highlight_length =
//                 issue.loc.last_index - issue.loc.first_index - 1;
//
//         ss << FMT::format("{} | {}{}^{}\n",
//                           std::string(line_box_width, ' '),      // Spaces pre  |
//                           std::string(visual_caret_offset, ' '), // spaces post | to move caret
//                           issue.pretty_color(), std::string(highlight_length, '~'));
//         ss << SGR_RESET;
//         if (issue.suggestion.has_value())
//         {
//             const auto raw_caret_offset_suggestion =
//                     issue.suggestion.value().insert_after.last_index -
//                     loc_tracker.find_index_of_line(
//                         loc_tracker.find_last_line(issue.suggestion.value().insert_after));
//             const auto visual_carret_offset_suggestion =
//                     compute_visual_caret_offset(line_views[i], raw_caret_offset_suggestion);
//             ss << FMT::format("{} | {}{}\n",
//                               std::string(line_box_width, ' '), // Spaces pre  |
//                               std::string(visual_carret_offset_suggestion + 1, ' '),
//                               // spaces post | to move caret
//                               issue.suggestion->text);
//         }
//         ss << SGR_RESET;
//     }
//
//     return ss.str();
// }
