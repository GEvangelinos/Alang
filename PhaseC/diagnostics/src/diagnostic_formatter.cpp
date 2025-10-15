#include "diagnostics/diagnostic_formatter.hpp"

#include <cctype>
#include <string>

#include "../../arguinator/include/arguinator/arguinator.hpp"
#include "core/source_location.hpp"
#include "diagnostics/diagnostic_types.hpp"
#include "support/cli_color.h"
#include "support/misc_tools.hpp"
#include "support/string_tools.hpp"

using HighlightTagID = std::size_t;
static_assert(
    std::is_unsigned_v<HighlightTagID>,
    "HighlightTagID must be unsigned: used in arithmetic operations (e.g. modulo for color selection)."
);

namespace
{
using namespace alpha;

class TaggedHighlight
{
public:
    TaggedHighlight(const Highlight *ref, HighlightTagID tag);
    auto ref() const noexcept { return DEBUG_REQUIRE_PTR(ref_); }
    auto tag() const noexcept { return tag_; }

private:
    const Highlight *ref_;
    HighlightTagID tag_;
};

TaggedHighlight::TaggedHighlight(
    const Highlight *const ref,
    const std::size_t tag)
    : ref_(DEBUG_REQUIRE_PTR(ref)), tag_(tag) {}

namespace sort_policy
{
    [[nodiscard]] bool
    leftmost_first(const TaggedHighlight &a, const TaggedHighlight &b)
    {
        return a.ref()->loc.begin < b.ref()->loc.begin;
    }

    [[nodiscard]] bool
    rightmost_first(const TaggedHighlight &a, const TaggedHighlight &b)
    {
        return a.ref()->loc.begin > b.ref()->loc.begin;
    }
} // namespace sort_policy

template<typename Predicate, typename Compare>
[[nodiscard]] std::vector<TaggedHighlight>
collect_line_highlight_tags(const Issue &issue, const Predicate should_collect, const Compare cmp)
{
    static_assert(
        std::is_invocable_r_v<bool, Predicate, const Highlight &>,
        "Predicate must be callable with (const Highlight &) and return bool"
    );
    static_assert(
        std::is_invocable_r_v<bool, Compare, const TaggedHighlight &, const TaggedHighlight &>,
        "Compare must be callable as bool(const TaggedHighlight&, const TaggedHighlight&)"
    );

    std::vector<TaggedHighlight> result;
    if (!issue.highlights)
        return result;

    HighlightTagID tag = 0;
    for (const Highlight &h : *issue.highlights)
    {
        if (should_collect(h))
            result.emplace_back(&h, tag);
        ++tag;
    }

    std::sort(result.begin(), result.end(), cmp);
    return result;
}

[[nodiscard]] bool
is_index_on_highlight(const SrcBufferIdx idx, const TaggedHighlight &hl)
{
    return hl.ref()->loc.begin <= idx && idx < hl.ref()->loc.end; // Reminder: loc.end exclusive.
}

[[nodiscard]] std::optional<HighlightTagID>
find_highlight_tag_at(const std::vector<TaggedHighlight> &highlights, const SrcBufferIdx idx)
{
    DEBUG_SMART_ASSERT(
        std::is_sorted(highlights.begin(),highlights.end(), &sort_policy::leftmost_first)&&
        "Invariant violation: 'highlights' must be sorted in ascending order by loc.begin\n"
        "(per sort_policy::leftmost_first) before calling current function."
    );

    for (const TaggedHighlight &hl : highlights)
        if (is_index_on_highlight(idx, hl))
            return hl.tag();
    return std::nullopt;
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
            issue.line(loc_tracker_).value,                                // {1} line
            issue.column(loc_tracker_).value,                              // {2} col
            apply_sgr(issue_type_color, to_string(issue.type), reset_sgr), // {3} issue type text
            apply_sgr(header_message_sgr, decorated_desc, reset_sgr)       // {4} description
        )
        << reset_sgr;
}

SrcBufferIdx
DiagnosticFormatter::find_end_of_code_in_line(const SrcBufferIdx line_start_idx) const
{
    SrcBufferIdx last_nonspace = line_start_idx;
    std::string codeline_accumulator;
    for (SrcBufferIdx idx = line_start_idx; ; ++idx)
    {
        const char ch = source_buffer_[idx.value];
        if (ch == '\0' || ch == '\n')
            break;

        codeline_accumulator.push_back(ch);
        if (!std::isspace(static_cast<unsigned char>(ch)))
            last_nonspace = idx;
    }
    const auto line_comment_pos = codeline_accumulator.find(Tokens::line_comment);
    if (line_comment_pos == std::string::npos)
        return last_nonspace;

    // -1 to move 1 chars before line comment token '//'
    const auto before_comment = line_start_idx.value + line_comment_pos - 1;
    DEBUG_SMART_ASSERT(support::is_in_numeric_range<SrcBufferIdx::UnderlyingType>(before_comment));
    return SrcBufferIdx{static_cast<SrcBufferIdx::UnderlyingType>(before_comment)};
}

std::string
DiagnosticFormatter::make_codeline(const Issue &issue, const SrcLineIdx line_no) const
{
    const auto touches_line = [line_no, this](const Highlight &hl)
    {
        const SrcLineIdx first = loc_tracker_.find_first_line(hl.loc);
        const SrcLineIdx last = loc_tracker_.find_last_line(hl.loc);
        return first <= line_no && line_no <= last;
    };
    std::vector<TaggedHighlight> touching_highlights =
        collect_line_highlight_tags(issue, touches_line, &sort_policy::rightmost_first);

    std::string codeline;
    SrcColumnIdx column{SrcColumnIdx::none};
    ToggleSwitch coloring_highlight{false};
    OnceFlag used_color;
    const SrcBufferIdx line_start_idx = loc_tracker_.find_index_of_line(line_no);
    for (SrcBufferIdx idx = line_start_idx; ; ++idx)
    {
        const char ch = source_buffer_[idx.value];
        if (ch == '\0' || ch == '\n')
            break;

        const bool should_start_coloring_highlight =
            !touching_highlights.empty() &&
            is_index_on_highlight(idx, touching_highlights.back()) &&
            coloring_highlight.is_disabled();
        const bool should_stop_coloring_highlight =
            !touching_highlights.empty() &&
            touching_highlights.back().ref()->loc.end == idx &&
            coloring_highlight.is_enabled();
        if (should_start_coloring_highlight)
        {
            codeline.append(get_highlight_color(touching_highlights.back().tag()));
            coloring_highlight.enable();
            if (!used_color) used_color.raise();
        }
        else if (should_stop_coloring_highlight)
        {
            codeline += SGR_RESET;
            coloring_highlight.disable();
            touching_highlights.pop_back();
        }
        const auto slots = ch == '\t' ? k_tab_width_ - column.value % k_tab_width_ : 1;
        column.value += slots;
        codeline.append(slots, ch == '\t' ? ' ' : ch);
    }
    auto result = support::rstrip(codeline); // We remove redundant suffix spaces.
    if (used_color)
        result.append(SGR_RESET);
    return result;
}

std::string
DiagnosticFormatter::make_underline(const Issue &issue, const SrcLineIdx line_no) const
{
    const auto touches_line = [line_no, this](const Highlight &hl)
    {
        const SrcLineIdx first = loc_tracker_.find_first_line(hl.loc);
        const SrcLineIdx last = loc_tracker_.find_last_line(hl.loc);
        return first <= line_no && line_no <= last;
    };
    std::vector<TaggedHighlight> touching_highlights =
        collect_line_highlight_tags(issue, touches_line, &sort_policy::rightmost_first);

    // Walk characters underlining primary issue (also expand tabs).
    std::string underline;
    SrcColumnIdx column{SrcColumnIdx::none};

    // Start index of line in source buffer.
    const SrcBufferIdx linestart_idx = loc_tracker_.find_index_of_line(line_no);
    const SrcBufferIdx end_of_code_idx = find_end_of_code_in_line(linestart_idx); // Inclusive

    OnceFlag seen_char;
    ToggleSwitch coloring_highlight{false};
    ToggleSwitch coloring_primary{false};
    OnceFlag used_color;
    for (SrcBufferIdx idx = linestart_idx; ; ++idx.value)
    {
        const char ch = source_buffer_[idx.value];
        if (ch == '\0' || ch == '\n' || idx > end_of_code_idx)
            break;

        const bool in_primary_issue = idx >= issue.loc.begin && idx < issue.loc.end;
        const bool should_start_coloring_highlight =
            !touching_highlights.empty() &&
            is_index_on_highlight(idx, touching_highlights.back()) &&
            coloring_highlight.is_disabled();
        const bool should_stop_coloring_highlight =
            !touching_highlights.empty() &&
            touching_highlights.back().ref()->loc.end == idx &&
            coloring_highlight.is_enabled();
        DEBUG_SMART_ASSERT(!(should_start_coloring_highlight && should_stop_coloring_highlight));
        const int slots = ch == '\t' ? k_tab_width_ - column.value % k_tab_width_ : 1;
        column.value += slots;
        if (should_start_coloring_highlight)
        {

            underline.append(get_highlight_color(touching_highlights.back().tag()));
            coloring_highlight.enable();
            if (coloring_primary.is_enabled())
                coloring_primary.disable();
            if (!used_color)
                used_color.raise();
        }
        else if (should_stop_coloring_highlight)
        {
            underline += SGR_RESET;
            coloring_highlight.disable();
            touching_highlights.pop_back();
        }
        if (!seen_char && !std::isspace(static_cast<unsigned char>(ch)))
            seen_char.raise();

        const bool under_code_section = seen_char && idx <= end_of_code_idx;
        if (coloring_highlight.is_enabled() && under_code_section)
            underline.append(slots, Markers::highlight);
        else if (in_primary_issue && under_code_section)
        {
            if (coloring_primary.is_disabled())
            {
                underline.append(get_underline_color(issue.type));
                coloring_primary.enable();
            }
            if (!underline_pointer_flag)
            {
                underline_pointer_flag = true;
                underline.append(1, Markers::pointer);
            }
            else
                underline.append(slots, Markers::underline);
        }
        else
            underline.append(slots, ' ');
    }
    if (used_color)
        underline.append(SGR_RESET);
    return underline;
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
DiagnosticFormatter::get_highlight_color(const HighlightTagID highlight_index) noexcept
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
    std::vector<TaggedHighlight> line_highlights =
        collect_line_highlight_tags(issue, starts_on_line, &sort_policy::leftmost_first);

    const auto leftmost_first = [](const TaggedHighlight &a, const TaggedHighlight &b)
    {
        return a.ref()->loc.begin < b.ref()->loc.begin;
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
            const auto hl_meta_id = curr_hl_meta.tag();
            const bool is_under_highlight_start_col
                = line_idx == curr_hl_meta.ref()->loc.begin;
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
                    apply_sgr(get_highlight_color(hl_meta_id), curr_hl_meta.ref()->desc,
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
            line_no.value,                                //{0}
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
