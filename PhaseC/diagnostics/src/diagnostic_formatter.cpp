#include "diagnostics/diagnostic_formatter.hpp"

#include <cctype>
#include <string>

#include "core/source_location.hpp"
#include "diagnostics/diagnostic_types.hpp"
#include "support/cli_color.h"
#include "support/misc_tools.hpp"
#include "support/string_tools.hpp"

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
} // namespace

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
      source_buffer_(support::require_ptr(source_buffer)),
      loc_tracker_(loc_tracker) {}

const char *
DiagnosticFormatter::get_underline_color(const Issue::Type type) noexcept
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

    const char *const header_location_sgr = colorize ? SGR_RESET COLOR_ASCII_BOLD_WHITE : "";
    const char *const header_message_sgr = colorize ? SGR_RESET COLOR_ASCII_WHITE : "";
    const char *const issue_type_color = colorize ? get_underline_color(issue.type) : "";
    const char *const backstick_section_sgr = SGR_RESET COLOR_ASCII_BOLD_WHITE;
    const char *const reset_sgr = colorize ? SGR_RESET : "";

    const auto prettify = [&](const std::string &s) -> std::string
    {
        std::string prettified;
        prettified.reserve(s.size() * 2); // Micro opt to reduce reallocations

        ToggleSwitch open_tick;
        for (const char ch : s)
        {
            if (ch != '`')
            {
                prettified += ch;
                continue;
            }
            if (open_tick.is_disabled())
            {
                prettified += ch;
                open_tick.enable();
                prettified += backstick_section_sgr;
            }
            else
            {
                open_tick.disable();
                prettified += header_message_sgr;
                prettified += ch;
            }
        }
        return prettified;
    };

    out << header_location_sgr;

    out << FMT::format(
        "{0}:{1}:{2}: {3}: {4}",
        source_filename_,                                              // {0} file
        line,                                                          // {1} line
        column,                                                        // {2} col
        apply_sgr(issue_type_color, to_string(issue.type), reset_sgr), // {3} issue type text
        apply_sgr(header_message_sgr, prettify(issue.desc), reset_sgr) // {4} description
    );
    out << reset_sgr;
}

std::string DiagnosticFormatter::build_codeline(const u32 line_no) const
{
    // Build codeline and also expand tabs
    std::string line;
    u32 column = 0;
    // Get the buffer index at which this line starts
    u32 i = loc_tracker_.find_index_of_line(line_no); // We start from start of line
    for (char ch; ((ch = source_buffer_[i])) && ch != '\n'; ++i)
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
    }
    return support::rstrip(line); // We remove redundant suffix spaces.
}

std::string
DiagnosticFormatter::build_underline(const Issue &issue, const u32 line_no) const
{
    std::vector<const Highlight *> line_highlights = get_highlights_of_line(issue,line_no);

    // Walk characters underlining primary issue (also expand tabs).
    std::string underline;
    u32 column = 0;
    u32 i = loc_tracker_.find_index_of_line(line_no); // Start index of line in source buffer.

    std::size_t idx_of_last_nonspace_in_line = std::string::npos;
    for (char ch; ((ch = source_buffer_[i])) && ch != '\n'; ++i)
        if (!std::isspace(static_cast<unsigned char>(ch)))
            idx_of_last_nonspace_in_line = i;

    bool seen_char = false;
    i = loc_tracker_.find_index_of_line(line_no); // Start index of line in source buffer.
    for (char ch; ((ch = source_buffer_[i])) && ch != '\n'; ++i)
    {
        const bool in_highlight_range = std::any_of(
            line_highlights.begin(), line_highlights.end(),
            [i](const Highlight *hl) { return i >= hl->loc.first_index && i < hl->loc.last_index; }
        );

        const bool in_primary_issue = i >= issue.loc.first_index && i < issue.loc.last_index;
        if (ch == '\t') // expand tab to spaces (based on its position)
        {
            const int spaces = k_tab_width_ - column % k_tab_width_;
            underline.append(spaces, ' ');
            column += spaces;
        }
        else
        {
            ++column;
            seen_char = seen_char || !std::isspace(static_cast<unsigned char>(ch));
            if (!(seen_char && i <= idx_of_last_nonspace_in_line))
                underline += ' ';
            else if (in_highlight_range)
                underline += highlight_marker;
            else if (in_primary_issue)
            {
                underline += !underline_pointer_flag ? pointer_marker : underline_marker;
                underline_pointer_flag = true;
            }
            else
                underline += ' ';
        }
    }
    return underline;
}

std::string
DiagnosticFormatter::colorize_highlights(
    const std::string &underline,
    const char *const highlight_color,
    const char *const underline_color)
{
    std::string colorized;
    colorized.reserve(underline.size() * 2); // Safe heuristic

    char prev = '\0';
    for (std::size_t i = 0; i < underline.size(); ++i)
    {
        const char curr = underline[i];
        if (curr == highlight_marker && prev != highlight_marker)
            colorized += highlight_color;
        if (prev == highlight_marker && curr != highlight_marker)
            colorized += underline_color;
        colorized += curr;
        prev = curr;
    }
    return colorized;
}

    void
DiagnosticFormatter:: build_highlight_labels(const Issue &issue, const u32 line_no)
{
    std::vector<const Highlight *> line_highlights = get_highlights_of_line(issue,line_no);

    std::sort(line_highlights.begin(), line_highlights.end(), [])


    // REMEMBER to expand TABS!

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
    const char *const underline_color = colorize ? get_underline_color(issue.type) : "";
    const char *const highlight_color = colorize ? COLOR_ASCII_YELLOW : "";
    const char *const reset_sgr = colorize ? SGR_RESET : "";

    const std::string codeline = build_codeline(line_no);
    std::string underline = build_underline(issue, line_no);
    underline = colorize_highlights(underline, highlight_color, underline_color);
    if (support::is_blank_str(codeline) &&
        support::is_blank_str(underline) &&
        !issue.suggestion.has_value())
        return;

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
            line_no,                                                                           //{0}
            k_linebox_width_,                                                                  //{1}
            codeline.substr(0, split_point),                                                   //{2}
            apply_sgr(suggestion_color, " " + issue.suggestion.value().desc + " ", reset_sgr), //{2}
            codeline.substr(split_point)                                                       //{4}
        );

        if (!underline.empty())
            out << FMT::format(
                "{0} | {1}\n",
                std::string(k_linebox_width_, ' '),                                      // {0}
                apply_sgr(underline_color, underline, reset_sgr) // {1}
            );
    }
    out << reset_sgr;
}

std::string
DiagnosticFormatter::format_issue(const Issue &issue, const bool colorize) const
{
    constexpr auto shown_part_size = max_shown_lines / 2;
    constexpr char ellipsis_block[] = "\t...\n\t...\n\t...\n";

    std::stringstream out;
    build_issue_header(out, issue, colorize);
    out << '\n';

    const Issue::RenderingLineSpan span = issue.compute_printing_span(loc_tracker_);

    underline_pointer_flag = false;
    const auto issue_line_count = span.end_line - span.start_line;
    if (issue_line_count < max_shown_lines)
        for (u32 line_no = span.start_line; line_no <= span.end_line; ++line_no)
            format_issue_line(out, issue, line_no, colorize);
    else
    {
        // Print leading context
        for (u32 line_no = span.start_line; line_no <= span.start_line + shown_part_size; ++line_no)
            format_issue_line(out, issue, line_no, colorize);
        // Ellipsis block to indicate omitted middle context.
        out << ellipsis_block;
        // Print trailing context
        for (u32 line_no = span.end_line - shown_part_size; line_no <= span.end_line; ++line_no)
            format_issue_line(out, issue, line_no, colorize);
    }

    return out.str();
}

 std::vector<const Highlight *>
DiagnosticFormatter::get_highlights_of_line(const Issue &issue, const u32 line_no) const
{
    std::vector<const Highlight *> line_highlights;
    if (issue.highlights.has_value())
        for (const Highlight &hl : *issue.highlights)
            if (loc_tracker_.find_first_line(hl.loc) == line_no || loc_tracker_.
                find_last_line(hl.loc) == line_no)
                line_highlights.push_back(&hl);
    return line_highlights;
}

std::string
DiagnosticFormatter::format_diagnostic(const Diagnostic &diagnostic, const bool colorize) const
{
    std::stringstream ss;
    ss << format_issue(diagnostic.primary, colorize);
    for (const Note &note : diagnostic.note_list)
        ss << format_issue(note, colorize);
    return ss.str();
}
} // namespace alpha
