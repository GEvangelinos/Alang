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

void
DiagnosticFormatter::build_issue_header(
    std::stringstream &out,
    const Issue &issue,
    const bool colorize) const
{
    const auto sgr = [colorize](const char *color) { return colorize ? color : ""; };
    const char *const header_location_sgr = sgr(SGR_RESET COLOR_FG_ASCII_BOLD_WHITE);
    const char *const header_message_sgr = sgr(SGR_RESET COLOR_FG_ASCII_WHITE);
    const char *const issue_type_color = sgr(get_underline_color(issue.type));
    const char *const decorated_sections_sgr = sgr(SGR_RESET COLOR_FG_ASCII_BOLD_WHITE);
    const char *const reset_sgr = sgr(SGR_RESET);

    const std::string decorated_desc = DiagnosticFormatter::decorate_sections(
        issue.desc,
        Markers::decorator,
        decorated_sections_sgr,
        header_message_sgr
    );

    out << header_location_sgr
        << FMT::format(
            "{0}:{1}:{2}: {3}: {4}",
            source_filename_,                                              // {0} file
            issue.line(loc_tracker_).value,                                      // {1} line
            issue.column(loc_tracker_).value,                                    // {2} col
            apply_sgr(issue_type_color, to_string(issue.type), reset_sgr), // {3} issue type text
            apply_sgr(header_message_sgr, decorated_desc, reset_sgr)       // {4} description
        )
        << reset_sgr;
}

SrcBufferIdx
DiagnosticFormatter::line_index_of_last_code_char(const SrcBufferIdx linestart_index) const
{
    // We start from start of line
    std::size_t idx_of_last_nonspace_in_line = std::string::npos;
    SrcBufferIdx i = linestart_index; // We start from start of line
    std::string string_canvas;
    for (char ch; ((ch = source_buffer_[i.value])) && ch != '\n'; ++i.value)
    {
        string_canvas.push_back(ch);
        if (!std::isspace(static_cast<unsigned char>(ch)))
            idx_of_last_nonspace_in_line = i.value;
    }
    const auto comment_pos = string_canvas.find("//"); // line comment token.
    if (comment_pos != std::string::npos)
    {
        // -1 cause idx is inclusive
        const auto before_comment_index = linestart_index.value + comment_pos - 1;
        DEBUG_SMART_ASSERT(
            support::is_in_numeric_range<SrcBufferIdx::UnderlyingType>(before_comment_index)
        );
        return SrcBufferIdx{static_cast<SrcBufferIdx::UnderlyingType>(before_comment_index)};
    }
    DEBUG_SMART_ASSERT(
        support::is_in_numeric_range<SrcBufferIdx::UnderlyingType>(idx_of_last_nonspace_in_line)
    );
    return SrcBufferIdx{static_cast<SrcBufferIdx::UnderlyingType>(idx_of_last_nonspace_in_line)};
}

std::string
DiagnosticFormatter::make_codeline(const Issue &issue, const SrcLineIdx line_no) const
{
    const auto touches_line = [line_no, this](const Highlight &hl)
    {
        const SrcLineIdx first = loc_tracker_.find_first_line(hl.loc);
        const SrcLineIdx last = loc_tracker_.find_last_line(hl.loc);
        return first <= line_no || line_no <= last;
    };

    std::vector<HighlightMeta> line_highlights = filter_highlights(issue, touches_line);

    const SrcBufferIdx linestart_idx = loc_tracker_.find_index_of_line(line_no);
    // We start from start of line
    SrcBufferIdx j = linestart_idx; // We start from start of line
    std::size_t idx_of_last_nonspace_in_line = std::string::npos;
    std::string string_canvas;
    for (char ch; ((ch = source_buffer_[j.value])) && ch != '\n'; ++j.value)
    {
        string_canvas.push_back(ch);
        if (!std::isspace(static_cast<unsigned char>(ch)))
            idx_of_last_nonspace_in_line = j.value;
    }
    const auto comment_pos = string_canvas.find("//"); // line comment token.
    if (comment_pos != std::string::npos)
        idx_of_last_nonspace_in_line = linestart_idx.value + comment_pos - 1;
    // -1 cause idx is inclusive

    // Build codeline and also expand tabs
    std::string line;
    u32 column = 0;
    // Get the buffer index at which this line starts

    SrcBufferIdx i = linestart_idx; // We start from start of line
    for (char ch; ((ch = source_buffer_[i.value])) && ch != '\n'; ++i.value)
    {
        const char *curr_hl_color = nullptr;
        const bool in_highlight_range = std::any_of(
            line_highlights.begin(), line_highlights.end(),
            [i, &curr_hl_color](const HighlightMeta &hl)
            {
                const bool result =
                    i >= hl.highlight()->loc.begin && i < hl.highlight()->loc.end;
                if (result)
                    curr_hl_color = get_highlight_color(hl.id());
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
            if (in_highlight_range && i.value <= idx_of_last_nonspace_in_line)
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
DiagnosticFormatter::make_underline(const Issue &issue, const SrcLineIdx line_no) const
{
    const auto touches_line = [line_no, this](const Highlight &hl)
    {
        const SrcLineIdx first = loc_tracker_.find_first_line(hl.loc);
        const SrcLineIdx last = loc_tracker_.find_last_line(hl.loc);
        return first <= line_no || line_no <= last;
    };

    std::vector<HighlightMeta> line_highlights = filter_highlights(issue, touches_line);

    // Walk characters underlining primary issue (also expand tabs).
    std::string underline;
    u32 column = 0;

    // Start index of line in source buffer.
    const SrcBufferIdx linestart_idx = loc_tracker_.find_index_of_line(line_no);
    SrcBufferIdx i = linestart_idx;

    std::size_t idx_of_last_nonspace_in_line = std::string::npos;
    std::string string_canvas;
    for (char ch; ((ch = source_buffer_[i.value])) && ch != '\n'; ++i.value)
    {
        string_canvas.push_back(ch);
        if (!std::isspace(static_cast<unsigned char>(ch)))
            idx_of_last_nonspace_in_line = i.value;
    }

    const auto comment_pos = string_canvas.find("//"); // line comment token.
    if (comment_pos != std::string::npos)
        idx_of_last_nonspace_in_line = linestart_idx.value + comment_pos - 1;
    // -1 cause idx is inclusive

    bool seen_char = false;
    i = linestart_idx;
    for (char ch; ((ch = source_buffer_[i.value])) && ch != '\n'; ++i.value)
    {
        const char *curr_hl_color = nullptr;
        const bool in_highlight_range = std::any_of(
            line_highlights.begin(), line_highlights.end(),
            [i, &curr_hl_color](const HighlightMeta &hl)
            {
                const bool result =
                    i >= hl.highlight()->loc.begin && i < hl.highlight()->loc.end;
                if (result)
                    curr_hl_color = get_highlight_color(hl.id());
                return result;
            }
        );

        const bool in_primary_issue = i >= issue.loc.begin && i < issue.loc.end;
        if (ch == '\t') // expand tab to spaces (based on its position)
        {
            const int spaces = k_tab_width_ - column % k_tab_width_;
            if (in_highlight_range)
            {
                DEBUG_SMART_ASSERT(!!curr_hl_color);
                underline.append(curr_hl_color);
                if (seen_char && i.value <= idx_of_last_nonspace_in_line)
                    underline.append(spaces, Markers::highlight);
                else
                    underline.append(spaces, ' ');
            }
            else if (in_primary_issue)
                underline.append(spaces, Markers::underline);
            else
                underline.append(spaces, ' ');
            column += spaces;
        }
        else
        {
            ++column;
            seen_char = seen_char || !std::isspace(static_cast<unsigned char>(ch));
            if (!(seen_char && i.value <= idx_of_last_nonspace_in_line))
                underline += ' ';
            else if (in_highlight_range)
            {
                underline += curr_hl_color;
                underline += Markers::highlight;
            }
            else if (in_primary_issue)
            {
                underline += !underline_pointer_flag ? Markers::pointer : Markers::underline;
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
        if (prev == Markers::highlight && curr != Markers::highlight)
            colorized += underline_color;
        colorized += curr;
        prev = curr;
    }
    return colorized;
}

std::string
DiagnosticFormatter::colorize_line_comment(const std::string_view codeline)
{
    const auto line_comment_pos = codeline.find(Tokens::line_comment);
    if (line_comment_pos == std::string::npos)
        return std::string(codeline);
    std::string colored(codeline.substr(0, line_comment_pos));
    colored += Colors::comment_color;
    colored += codeline.substr(line_comment_pos);
    colored += SGR_RESET;
    return colored;
}

void
DiagnosticFormatter::swap_markers(std::string &str, const char old_marker, const char new_marker)
{
    std::replace(str.begin(), str.end(), old_marker, new_marker);
}

const char *
DiagnosticFormatter::get_underline_color(const Issue::Type type) noexcept
{
    using IT = Issue::Type;
    switch (type)
    {
    case IT::NOTE: return Colors::note_fg;
    case IT::WARNING: return Colors::warning_fg;
    case IT::SOFT_ERROR:
    case IT::HARD_ERROR:
    case IT::FATAL_ERROR: return Colors::error_fg;
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

std::string
DiagnosticFormatter::decorate_sections(
    const std::string_view str,
    const char marker,
    const std::string_view sgr_section_prefix,
    const std::string_view sgr_section_suffix)
{
    std::string decorated;
    decorated.reserve(str.size() * 2); // Micro opt to reduce reallocations

    ToggleSwitch seen_marker;
    for (const char ch : str)
    {
        if (ch != marker)
        {
            decorated += ch;
            continue;
        }
        if (seen_marker.is_disabled())
        {
            seen_marker.enable();
            decorated += ch;
            decorated += sgr_section_prefix;
        }
        else
        {
            seen_marker.disable();
            decorated += sgr_section_suffix;
            decorated += ch;
        }
    }
    return decorated;
}

std::vector<std::string>
DiagnosticFormatter::build_highlight_labels(const Issue &issue, const SrcLineIdx line_no) const
{
    const auto starts_on_line = [line_no, this](const Highlight &hl)
    {
        return loc_tracker_.find_first_line(hl.loc) == line_no;
    };
    std::vector<HighlightMeta> line_highlights = filter_highlights(issue, starts_on_line);

    const auto leftmost_first = [](const HighlightMeta &a, const HighlightMeta &b)
    {
        return a.highlight()->loc.begin < b.highlight()->loc.begin;
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
        for (char ch; ((ch = source_buffer_[line_idx.value])) && ch != '\n'; ++line_idx)
        {
            if (hl_idx == line_highlights.size())
                break;
            const auto &curr_hl_meta = line_highlights[hl_idx];
            const auto hl_meta_id = curr_hl_meta.id();
            const bool is_under_highlight_start_col
                = line_idx == curr_hl_meta.highlight()->loc.begin;
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
                    apply_sgr(get_highlight_color(hl_meta_id), curr_hl_meta.highlight()->desc,
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
    const SrcLineIdx line_no = loc_tracker_.find_last_line(suggestion.insert_after);
    const SrcBufferIdx line_start_index = loc_tracker_.find_index_of_line(line_no);

    DEBUG_SMART_ASSERT(suggestion.insert_after.end >= line_start_index);

    u32 column = 0;
    // instead of computing suggestion width with plain subtraction
    // we walk the input buffer in case there is a tab to expand.
    for (auto i = line_start_index; i < suggestion.insert_after.end; ++i)
    {
        const char ch = source_buffer_[i.value];
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
    const SrcLineIdx line_no,
    const bool colorize) const
{
    DEBUG_SMART_ASSERT(line_no.value > 0 && "Line number is invalid (lines start at 1).");

    const char *const suggestion_color = colorize ? COLOR_FG_ASCII_GREEN : "";
    const char *const underline_color = colorize ? get_underline_color(issue.type) : "";
    const char *const reset_sgr = colorize ? SGR_RESET : "";

    std::string codeline = make_codeline(issue, line_no);
    codeline = colorize_line_comment(codeline);
    std::string underline = make_underline(issue, line_no);
    underline = colorize_underline(underline, underline_color);
    swap_markers(underline, Markers::highlight, Markers::underline);
    if (support::is_blank_str(codeline) &&
        support::is_blank_str(underline) &&
        !issue.suggestion.has_value())
        return;

    SrcLineIdx suggestion_line_no{SrcLineIdx::none};
    if (issue.suggestion.has_value())
        suggestion_line_no = loc_tracker_.find_last_line(issue.suggestion->insert_after);
    if (!issue.suggestion.has_value() || suggestion_line_no != line_no)
    {
        out << FMT::format("{0:>{1}} | {2}\n", line_no.value, k_linebox_width_, codeline);
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
            line_no.value,                                      //{0}
            k_linebox_width_,                             //{1}
            codeline.substr(0, split_point),              //{2}
            apply_sgr(suggestion_color, "  ", reset_sgr), //{2}
            codeline.substr(split_point)                  //{4}
        );

        if (!underline.empty())
            out << FMT::format(
                "{0:>{1}} | {2}\n",
                "",               //{0}
                k_linebox_width_, //{1}
                apply_sgr(suggestion_color, std::string(split_point, ' ') + " " + "^",
                          reset_sgr) // {2}
            );

        if (!underline.empty())
            out << FMT::format(
                "{0:>{1}} | {2}\n",
                "",                                                                          //{0}
                k_linebox_width_,                                                            //{1}
                apply_sgr(suggestion_color, std::string(split_point, ' ') + " |", reset_sgr) // {2}
            );
        if (!underline.empty())
            out << FMT::format(
                "{0:>{1}} | {2}\n",
                "",               //{0}
                k_linebox_width_, //{1}
                apply_sgr(suggestion_color,
                          std::string(split_point, ' ') + " " + issue.suggestion->desc,
                          reset_sgr) // {2}
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

    std::stringstream out;
    build_issue_header(out, issue, colorize);
    out << '\n';

    const Issue::RenderingLineSpan span = issue.compute_printing_span(loc_tracker_);

    underline_pointer_flag = false;
    const auto issue_line_count = span.end_line.value - span.begin_line.value;
    if (issue_line_count < max_shown_lines)
        for (auto line_no = span.begin_line; line_no <= span.end_line; ++line_no)
            format_issue_line(out, issue, line_no, colorize);
    else
    {
        // Print leading context
        for (auto line_no = span.begin_line;
             line_no.value <= span.begin_line.value + shown_part_size;
             ++line_no)
            format_issue_line(out, issue, line_no, colorize);

        // Ellipsis block to indicate omitted middle context.
        out << ellipsis_block;

        // Print trailing context

        for (auto line_no = SrcLineIdx{span.end_line.value - shown_part_size};
             line_no <= span.end_line;
             ++line_no)
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
    : highlight_(DEBUG_REQUIRE_PTR(highlight)), id_(id) {}
} // namespace alpha
