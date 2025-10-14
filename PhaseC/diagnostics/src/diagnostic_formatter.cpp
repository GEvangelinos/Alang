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

const char *
DiagnosticFormatter::get_highlight_color(const std::size_t highlight_index) noexcept
{
    static constexpr const char *highlight_colors[] = {
        COLOR_FG_PINK,
        COLOR_FG_SKY,
        COLOR_FG_MINT,
    };
    return highlight_colors[highlight_index % std::size(highlight_colors)];
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

std::string DiagnosticFormatter::build_codeline(const Issue &issue, const u32 line_no) const
{
    const auto touches_line = [line_no, this](const Highlight &hl)
    {
        const auto first = loc_tracker_.find_first_line(hl.loc);
        const auto last = loc_tracker_.find_last_line(hl.loc);
        return first <= line_no || line_no <= last;
    };

    std::vector<HighlightMeta> line_highlights = filter_highlights(issue, touches_line);

    const u32 linestart_idx = loc_tracker_.find_index_of_line(line_no);
    // We start from start of line
    u32 j = linestart_idx; // We start from start of line
    std::size_t idx_of_last_nonspace_in_line = std::string::npos;
    std::string string_canvas;
    for (char ch; ((ch = source_buffer_[j])) && ch != '\n'; ++j)
    {
        string_canvas.push_back(ch);
        if (!std::isspace(static_cast<unsigned char>(ch)))
            idx_of_last_nonspace_in_line = j;
    }
    const auto comment_pos = string_canvas.find("//"); // line comment token.
    if (comment_pos != std::string::npos)
        idx_of_last_nonspace_in_line = linestart_idx + comment_pos - 1; // -1 cause idx is inclusive

    // Build codeline and also expand tabs
    std::string line;
    u32 column = 0;
    // Get the buffer index at which this line starts

    u32 i = linestart_idx; // We start from start of line
    for (char ch; ((ch = source_buffer_[i])) && ch != '\n'; ++i)
    {
        const char *curr_hl_color = nullptr;
        const bool in_highlight_range = std::any_of(
            line_highlights.begin(), line_highlights.end(),
            [i, &curr_hl_color](const HighlightMeta &hl)
            {
                const bool result =
                    i >= hl.highlight->loc.first_index && i < hl.highlight->loc.last_index;
                if (result)
                    curr_hl_color = get_highlight_color(hl.id);
                return result;
            }
        );
        if (ch == '\t')
        {
            const int spaces = k_tab_width_ - column % k_tab_width_;
            line.append(spaces, ' ');
            column += spaces;
        }
        else
        {
            if (in_highlight_range && i <= idx_of_last_nonspace_in_line)
            {
                line += curr_hl_color;
                line += ch;
                line += SGR_RESET;
            }
            else
                line += ch;
            ++column;
        }
    }
    return support::rstrip(line); // We remove redundant suffix spaces.
}

std::string
DiagnosticFormatter::build_underline(const Issue &issue, const u32 line_no) const
{
    const auto touches_line = [line_no, this](const Highlight &hl)
    {
        const auto first = loc_tracker_.find_first_line(hl.loc);
        const auto last = loc_tracker_.find_last_line(hl.loc);
        return first <= line_no || line_no <= last;
    };

    std::vector<HighlightMeta> line_highlights = filter_highlights(issue, touches_line);

    // Walk characters underlining primary issue (also expand tabs).
    std::string underline;
    u32 column = 0;

    // Start index of line in source buffer.
    const u32 linestart_idx = loc_tracker_.find_index_of_line(line_no);
    u32 i = linestart_idx;


    std::size_t idx_of_last_nonspace_in_line = std::string::npos;
    std::string string_canvas;
    for (char ch; ((ch = source_buffer_[i])) && ch != '\n'; ++i)
    {
        string_canvas.push_back(ch);
        if (!std::isspace(static_cast<unsigned char>(ch)))
            idx_of_last_nonspace_in_line = i;
    }

    const auto comment_pos = string_canvas.find("//"); // line comment token.
    if (comment_pos != std::string::npos)
        idx_of_last_nonspace_in_line = linestart_idx + comment_pos - 1; // -1 cause idx is inclusive

    bool seen_char = false;
    i = linestart_idx;
    for (char ch; ((ch = source_buffer_[i])) && ch != '\n'; ++i)
    {
        const char *curr_hl_color = nullptr;
        const bool in_highlight_range = std::any_of(
            line_highlights.begin(), line_highlights.end(),
            [i, &curr_hl_color](const HighlightMeta &hl)
            {
                const bool result =
                    i >= hl.highlight->loc.first_index && i < hl.highlight->loc.last_index;
                if (result)
                    curr_hl_color = get_highlight_color(hl.id);
                return result;
            }
        );

        const bool in_primary_issue = i >= issue.loc.first_index && i < issue.loc.last_index;
        if (ch == '\t') // expand tab to spaces (based on its position)
        {
            const int spaces = k_tab_width_ - column % k_tab_width_;
            if (in_highlight_range)
            {
                DEBUG_SMART_ASSERT(!!curr_hl_color);
                underline.append(curr_hl_color);
                if (seen_char && i <= idx_of_last_nonspace_in_line)
                    underline.append(spaces, highlight_marker);
                else
                    underline.append(spaces, ' ');
            }
            else if (in_primary_issue)
                underline.append(spaces, underline_marker);
            else
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
            {
                underline += curr_hl_color;
                underline += highlight_marker;
            }
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
DiagnosticFormatter::colorize_underline(
    const std::string &underline,
    const char *const underline_color)
{
    std::string colorized;
    colorized.reserve(underline.size() * 2); // Safe heuristic

    char prev = '\0';
    for (std::size_t i = 0; i < underline.size(); ++i)
    {
        const char curr = underline[i];
        if (prev == highlight_marker && curr != highlight_marker)
            colorized += underline_color;
        colorized += curr;
        prev = curr;
    }
    return colorized;
}

std::string
DiagnosticFormatter::colorize_codeline(const std::string &codeline)
{
    const auto line_comment_pos = codeline.find("//");
    if (line_comment_pos == std::string::npos)
        return codeline;
    std::string colored = codeline.substr(0, line_comment_pos);
    colored += comment_color;
    colored += codeline.substr(line_comment_pos);
    colored += SGR_RESET;
    return colored;
}

void
DiagnosticFormatter::swap_highlight_marker(std::string &underline)
{
    std::replace(underline.begin(), underline.end(), highlight_marker, underline_marker);
}

std::vector<std::string>
DiagnosticFormatter::build_highlight_labels(const Issue &issue, const u32 line_no) const
{
    const auto starts_on_line = [line_no, this](const Highlight &hl)
    {
        return loc_tracker_.find_first_line(hl.loc) == line_no;
    };
    std::vector<HighlightMeta> line_highlights = filter_highlights(issue, starts_on_line);

    const auto leftmost_first = [](const HighlightMeta &a, const HighlightMeta &b)
    {
        return a.highlight->loc.first_index < b.highlight->loc.first_index;
    };

    std::sort(line_highlights.begin(), line_highlights.end(), leftmost_first);

    std::vector<std::string> label_lines;

    const auto initial_line_inx = loc_tracker_.find_index_of_line(line_no);
    for (auto j = line_highlights.size() + 1; j > 0; --j)
    {
        auto line_idx = initial_line_inx;
        std::string current_label_line{};
        std::size_t hl_idx = 0;
        std::size_t column = 0;
        for (char ch; ((ch = source_buffer_[line_idx])) && ch != '\n'; ++line_idx)
        {
            if (hl_idx == line_highlights.size())
                break;
            const auto &curr_hl_meta = line_highlights[hl_idx];
            const auto hl_meta_id = curr_hl_meta.id;
            const bool is_under_highlight_start_col
                = line_idx == curr_hl_meta.highlight->loc.first_index;
            if (ch == '\t') // expand tab to spaces (based on its position)
            {
                const int spaces = k_tab_width_ - column % k_tab_width_;
                current_label_line.append(spaces - 1, ' ');
                column += spaces;
            }
            else
                ++column;
            if (is_under_highlight_start_col && hl_idx + 1 == j)
            {
                current_label_line +=
                    apply_sgr(get_highlight_color(hl_meta_id), curr_hl_meta.highlight->desc,
                              SGR_RESET);

                break;
            }
            if (is_under_highlight_start_col)
            {
                current_label_line += apply_sgr(get_highlight_color(hl_meta_id), "|", SGR_RESET);
                ++hl_idx;
            }
            else
                current_label_line += ' ';
        }
        label_lines.push_back(std::move(current_label_line));
        current_label_line.clear();
    }

    return label_lines;
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

    const char *const suggestion_marker = "^";
    const char *const suggestion_color = colorize ? COLOR_ASCII_GREEN : "";
    const char *const underline_color = colorize ? get_underline_color(issue.type) : "";
    const char *const reset_sgr = colorize ? SGR_RESET : "";

    std::string codeline = build_codeline(issue, line_no);
    codeline = colorize_codeline(codeline);
    std::string underline = build_underline(issue, line_no);
    underline = colorize_underline(underline, underline_color);
    swap_highlight_marker(underline);
    if (support::is_blank_str(codeline) &&
        support::is_blank_str(underline) &&
        !issue.suggestion.has_value())
        return;

    u32 suggestion_line_no = 0;
    if (issue.suggestion.has_value())
        suggestion_line_no = loc_tracker_.find_last_line(issue.suggestion->insert_after);
    if (!issue.suggestion.has_value() || suggestion_line_no != line_no)
    {
        out << FMT::format("{0:>{1}} | {2}\n", line_no, k_linebox_width_, codeline);
        if (!underline.empty())
            out << FMT::format(
                "{0:>{1}} | {2}\n",
                "",                                              //{0}
                k_linebox_width_,                                //{1}
                apply_sgr(underline_color, underline, reset_sgr) // {2}
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
            apply_sgr(suggestion_color, "  ", reset_sgr), //{2}
            codeline.substr(split_point)                                                       //{4}
        );

        if (!underline.empty())
            out << FMT::format(
                "{0:>{1}} | {2}\n",
                "",                                              //{0}
                k_linebox_width_,                                //{1}
                apply_sgr(suggestion_color, std::string(split_point, ' ') +" "+ "^" , reset_sgr) // {2}
            );

        if (!underline.empty())
            out << FMT::format(
                "{0:>{1}} | {2}\n",
                "",                                              //{0}
                k_linebox_width_,                                //{1}
                apply_sgr(suggestion_color, std::string(split_point, ' ') + " |", reset_sgr) // {2}
            );
        if (!underline.empty())
            out << FMT::format(
                "{0:>{1}} | {2}\n",
                "",                                              //{0}
                k_linebox_width_,                                //{1}
                apply_sgr(suggestion_color, std::string(split_point, ' ') +" " + issue.suggestion->desc, reset_sgr) // {2}
            );
    }
    const std::vector<std::string> label_lines = build_highlight_labels(issue, line_no);
    std::string label_box{};
    for (const auto &line : label_lines)
    {
        if (support::is_blank_str(line))
            continue;

        label_box += FMT::format(
            "{0:>{1}} | {2}\n",
            "",               //{0}
            k_linebox_width_, //{1}
            line              //{2}
        );
    }
    out << label_box;
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

std::string
DiagnosticFormatter::format_diagnostic(const Diagnostic &diagnostic, const bool colorize) const
{
    std::stringstream ss;
    ss << format_issue(diagnostic.primary, colorize);
    for (const Note &note : diagnostic.note_list)
        ss << format_issue(note, colorize);
    return ss.str();
}

DiagnosticFormatter::HighlightMeta::HighlightMeta(
    const Highlight *const highlight,
    const std::size_t id)
    : highlight(DEBUG_REQUIRE_PTR(highlight)), id(id) {}
} // namespace alpha
