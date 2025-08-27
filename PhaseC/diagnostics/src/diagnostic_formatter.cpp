#include "diagnostics/diagnostic_formatter.hpp"

#include <cctype>
#include <string>

#include "core/source_location.hpp"
#include "diagnostics/diagnostic_types.hpp"
#include "utils/cli_color.h"
#include "utils/misc.hpp"

namespace
{
[[maybe_unused, deprecated("Used in old diagnostic system")]]
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

std::string DiagnosticFormatter::apply_sgr(
    const std::string_view prefix,
    const std::string_view text,
    const std::string_view suffix)
{
    std::string out;
    out.reserve(prefix.size() + text.size() + suffix.size());
    out.append(prefix).append(text).append(suffix);
    return out;
}

void
DiagnosticFormatter::build_issue_header(
    std::stringstream &out,
    const Issue &issue,
    const bool colorize) const
{
    const u32 line = issue.line(loc_tracker_);
    const u32 column = issue.column(loc_tracker_);

    const char *const header_sgr = colorize ? COLOR_ASCII_BOLD_DEFAULT : "";
    const char *const issue_type_color = colorize ? highlight_color(issue.type) : "";
    const char *const reset_sgr = colorize ? SGR_RESET : "";

    out << header_sgr;

    out << FMT::format(
        "{0}:{1}:{2}: {3}: {4}",
        source_filename_,                                              // {0} file
        line,                                                          // {1} line
        column,                                                        // {2} col
        apply_sgr(issue_type_color, to_string(issue.type), reset_sgr), // {3} issue type text
        apply_sgr(header_sgr, issue.desc, reset_sgr)                   // {4} description
    );
    out << reset_sgr;
}

std::string DiagnosticFormatter::build_codeline(const u32 line_no) const
{
    std::string line;

    // Build codeline and also expand tabs
    u32 i = loc_tracker_.find_index_of_line(line_no);
    u32 column = 0;
    char ch;
    while ((ch = source_buffer_[i]) != '\n' && ch != '\0')
    {
        if (ch == '\t')
        {
            const int spaces = k_tab_width_ - column % k_tab_width_;
            line.append(spaces, ' ');
            column += spaces;
        }
        else
        {
            line += ch;
            ++column;
        }
        ++i;
    }
    return line;
}

std::string
DiagnosticFormatter::build_underline(const Issue &issue, const u32 line_no) const
{
    std::string underline;

    // Get the buffer index at which this line starts
    const u32 line_start = loc_tracker_.find_index_of_line(line_no);

    u32 column = 0;
    // Walk characters until newline, building a highlight string (also expand tabs).
    for (auto idx = line_start; source_buffer_[idx] != '\n'; ++idx)
    {
        const bool outside_issue = idx < issue.loc.first_index || idx >= issue.loc.last_index;
        const char ch = source_buffer_[idx];
        if (ch == '\t') // expand tab to spaces (based on its position)
        {
            const int spaces = k_tab_width_ - column % k_tab_width_;
            underline.append(spaces, ' ');
            column += spaces;
        }
        else if (outside_issue || std::isspace(static_cast<unsigned char>(ch)))
        {
            underline += ' ';
            ++column;
        }
        else
        {
            ++column;
            if (highlight_pointer_flag.is_enabled())
            {
                underline += pointer_marker;
                highlight_pointer_flag.disable();
            }
            else
                underline += underline_marker;
        }
    }
    return underline;
}

std::vector<std::string>
DiagnosticFormatter::build_suggestion_lines(const Suggestion &suggestion) const
{
    const u32 line_no = loc_tracker_.find_last_line(suggestion.insert_after);
    const u32 line_start = loc_tracker_.find_index_of_line(line_no);

    DEBUG_SMART_ASSERT(suggestion.insert_after.last_index >= line_start);

    // How far into the line the suggestion should be indented,
    // so that first character of each line is under suggested source location.
    const u32 indent_width = compute_visual_suggestion_indent_width(suggestion);
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

// Number of columns (spaces) to indent the suggestion so its first char
// appears immediately after insert_after.last_index on this line.
u32
DiagnosticFormatter::compute_visual_suggestion_indent_width(const Suggestion &suggestion) const
{
    const u32 line_no = loc_tracker_.find_last_line(suggestion.insert_after);
    const u32 line_start_index = loc_tracker_.find_index_of_line(line_no);

    DEBUG_SMART_ASSERT(suggestion.insert_after.last_index >= line_start_index);

    u32 column = 0;
    // instead of computing suggestion width with plain subtraction
    // we walk the input buffer in case there is a tab to expand.
    for (auto i = line_start_index; i < suggestion.insert_after.last_index; ++i)
    {
        const char ch = source_buffer_[i];
        if (ch == '\t')
            column += k_tab_width_ - column % k_tab_width_;
        else
            ++column;
    }

    return column;
}

void
DiagnosticFormatter::format_issue_line(
    std::stringstream &out,
    const Issue &issue,
    const u32 line_no,
    const bool colorize) const
{
    DEBUG_SMART_ASSERT(line_no > 0 && "Line number is invalid (lines start at 1).");

    const char *const suggestion_marker = " ";
    const char *const suggestion_color = colorize ? COLOR_ASCII_GREEN : "";
    const char *const underline_color = colorize ? highlight_color(issue.type) : "";
    const char *const reset_sgr = colorize ? SGR_RESET : "";

    const std::string codeline = build_codeline(line_no);
    const std::string underline = build_underline(issue, line_no);

    u32 suggestion_line_no = 0;
    if (issue.suggestion.has_value())
        suggestion_line_no = loc_tracker_.find_last_line(issue.suggestion->insert_after);
    if (!issue.suggestion.has_value() || suggestion_line_no != line_no)
    {
        out << FMT::format("{:>{}} | {}\n", line_no, k_linebox_width_, codeline);
        if (!underline.empty())
            out << FMT::format(
                "{0} | {1}\n",
                std::string(k_linebox_width_, ' '),              // {0}
                apply_sgr(underline_color, underline, reset_sgr) // {1}
            );
    }
    else
    {
        DEBUG_SMART_ASSERT(issue.suggestion.has_value());
        const u32 split_point = compute_visual_suggestion_indent_width(issue.suggestion.value());

        out << FMT::format(
            "{0:>{1}} | {2}{3}{4}\n",
            line_no,                         // {0}
            k_linebox_width_,                // {1}
            codeline.substr(0, split_point), // {2}
            suggestion_marker,               // {3}
            codeline.substr(split_point)     // {4}
        );

        if (!underline.empty())
            out << FMT::format(
                "{0} | {1}{2}{3}\n",
                std::string(k_linebox_width_, ' '),                                      // {0}
                apply_sgr(underline_color, underline.substr(0, split_point), reset_sgr), // {1}
                apply_sgr(suggestion_color, "^", reset_sgr),                             // {2}
                apply_sgr(underline_color, underline.substr(split_point), reset_sgr)     // {1}
            );
        out << FMT::format(
            "{0} | {1}{2}\n",
            std::string(k_linebox_width_, ' '),         // {0}
            std::string(split_point, ' '),              // {1}
            apply_sgr(suggestion_color, "|", reset_sgr) // {2}
        );
        const auto suggestion_lines = build_suggestion_lines(issue.suggestion.value());
        for (const auto &sl: suggestion_lines)
            out << FMT::format(
                "{0} | {1}\n",
                std::string(k_linebox_width_, ' '),        // {0}
                apply_sgr(suggestion_color, sl, reset_sgr) // {1}
            );
    }
    out << reset_sgr;
}

std::string
DiagnosticFormatter::format_issue(const Issue &issue, const bool colorize) const
{
    std::stringstream out;
    build_issue_header(out, issue, colorize);
    out << '\n';

    highlight_pointer_flag.enable();
    const Issue::RenderingSpan span = issue.compute_printing_span(loc_tracker_);
    for (u32 line_no = span.start_line; line_no <= span.end_line; ++line_no)
        format_issue_line(out, issue, line_no, colorize);

    return out.str();
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
