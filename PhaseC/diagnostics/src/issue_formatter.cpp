#include "diagnostics/issue_formatter.hpp"

#include <sstream>

#include "core/translation_unit_buffer.hpp"
#include "support/cli_color.h"
#include "support/misc_tools.hpp"
#include "support/string_tools.hpp"

namespace
{
using namespace alpha;

using HighlightTag = std::size_t;

class TaggedHighlight
{
public:
    TaggedHighlight(const Highlight* const ref, const HighlightTag tag)
        : ref_(DEBUG_REQUIRE_PTR(ref)), tag_(tag) {}

    [[nodiscard]] auto ref() const noexcept { return ref_; }
    [[nodiscard]] auto tag() const noexcept { return tag_; }

private:
    const Highlight* ref_;
    HighlightTag tag_;
};

namespace sort_policy
{
    [[nodiscard]] bool leftmost_first(const TaggedHighlight& a, const TaggedHighlight& b);
    [[nodiscard]] bool rightmost_first(const TaggedHighlight& a, const TaggedHighlight& b);
} // namespace sort_policy

void swap_markers(std::string& str, char old_marker, char new_marker);
[[nodiscard]] bool is_index_on_highlight(SrcBuffIdx idx, const TaggedHighlight& hl);
[[nodiscard]] bool is_index_on_start_of_highlight(SrcBuffIdx idx, const TaggedHighlight& hl);
[[nodiscard]] std::optional<HighlightTag>
find_highlight_tag_at(const std::vector<TaggedHighlight>& highlights, SrcBuffIdx idx);
[[nodiscard]] std::string
apply_sgr(std::string_view prefix, std::string_view text, std::string_view suffix);
} // namespace

namespace alpha
{
class IssueFormatterImpl final
{
public:
    IssueFormatterImpl(
        const TranslationUnitBuffer& source_buffer,
        const LocationTracker& loc_tracker,
        const Issue& target,
        bool colorize);

    [[nodiscard]] std::string format(std::filesystem::path source_path);

private:
    const TranslationUnitBuffer& source_buffer_;
    const LocationTracker& loc_tracker_;
    const Issue& target_;
    const Issue::RenderingLineSpan rendering_span_;
    const bool colorize_;
    SrcLineIdx working_line_;
    ToggleSwitch coloring_primary_;
    ToggleSwitch coloring_highlight_;
    ToggleSwitch colored_working_line_;
    OnceFlag primary_beginning_marked_;

    [[nodiscard]] std::string format_issue_line();
    [[nodiscard]] std::string make_issue_header(std::filesystem::path source_path) const;
    [[nodiscard]] std::string make_codeline();
    [[nodiscard]] std::string format_issue_line_with_suggestion(
        const std::string& codeline,
        const std::string& underline,
        const std::vector<std::string>& highlight_anchors,
        const std::vector<std::string>& highlight_labels);
    [[nodiscard]] std::string make_underline();
    [[nodiscard]] std::vector<std::string> make_highlight_anchors(
        std::size_t root_height, char anchor_marker);
    [[nodiscard]] std::vector<std::string> make_highlight_labels();
    [[nodiscard]] bool source_blank_afterwards(SrcBuffIdx idx) const noexcept;

    [[nodiscard]] SrcBuffIdx find_end_of_code_in_line(SrcBuffIdx line_start_idx) const;
    void ensure_primary_start_marked(std::string& underline);
    void handle_possible_coloring_start(
        std::string& line_accumulator,
        const std::vector<TaggedHighlight>& highlights,
        SrcBuffIdx idx,
        bool should_try_color_primary);
    void handle_possible_coloring_stop(
        std::string& line_accumulator,
        std::vector<TaggedHighlight>& highlights,
        SrcBuffIdx idx);

    void finalize_colored_line_accumulator(std::string& line_accumulator);

    template <typename Predicate, typename Compare>
    [[nodiscard]] std::vector<TaggedHighlight>
    collect_line_highlight_tags(Predicate should_collect, Compare cmp) const;

    [[nodiscard]] static std::string decorate_sections(
        std::string_view str,
        char marker,
        std::string_view sgr_section_prefix,
        std::string_view sgr_section_suffix,
        bool keep_decorator_marker = true);
    [[nodiscard]] static std::string colorize_line_comment(std::string_view codeline);
    [[nodiscard]] SrcColumnIdx compute_visual_suggestion_indent_width(
        const Suggestion& suggestion) const;

    [[nodiscard]] const char* sgr(const char* const color) const noexcept
    {
        return colorize_ ? color : "";
    }

    [[nodiscard]] static Word calculate_slots(SrcColumnIdx& column_idx, char ch);
};

IssueFormatter::IssueFormatter(
    const std::filesystem::path source_path,
    const TranslationUnitBuffer& source_buffer,
    const LocationTracker& loc_tracker,
    const bool colorize)
    : source_path_(source_path),
      source_buffer_(source_buffer),
      loc_tracker_(loc_tracker),
      colorize_(colorize) {}

std::string
IssueFormatter::format(const Issue& issue)
{
    IssueFormatterImpl impl_{
        source_buffer_,
        loc_tracker_,
        issue,
        colorize_
    };
    return impl_.format(source_path_);
}

const char*
IssueFormatter::get_underline_color(const Issue::Type type) noexcept
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

const char*
IssueFormatter::get_highlight_color(const std::size_t highlight_index) noexcept
{
    static constexpr const char* highlight_colors[] = {
        COLOR_FG_PINK,
        COLOR_FG_SKY,
        COLOR_FG_MINT,
    };
    return highlight_colors[highlight_index % std::size(highlight_colors)];
}

IssueFormatterImpl::IssueFormatterImpl(
    const TranslationUnitBuffer& source_buffer,
    const LocationTracker& loc_tracker,
    const Issue& target,
    const bool colorize)
    : source_buffer_(source_buffer),
      loc_tracker_(loc_tracker),
      target_(target),
      rendering_span_(target.compute_rendering_span(loc_tracker)),
      colorize_(colorize),
      working_line_(rendering_span_.begin_line)
{
    DEBUG_SMART_ASSERT(working_line_.value > 0 && "Line number is invalid (lines start at 1).");
}

// Number of columns (spaces) to indent the suggestion so its first char
// appears immediately after insert_after.last_index on this line.
SrcColumnIdx
IssueFormatterImpl::compute_visual_suggestion_indent_width(const Suggestion& suggestion) const
{
    const SrcLineIdx line_no = loc_tracker_.find_last_line(suggestion.insert_after);
    const SrcBuffIdx line_start_index = loc_tracker_.find_index_of_line(line_no);

    DEBUG_SMART_ASSERT(suggestion.insert_after.end >= line_start_index);

    SrcColumnIdx column = SrcColumnIdx::none();
    // Instead of computing suggestion width with plain subtraction
    // we walk the input buffer in case there is a tab to expand.
    for (auto i = line_start_index; i < suggestion.insert_after.end; ++i)
    {
        const char ch = source_buffer_[i];
        if (ch == '\t')
            column.value += IssueFormatter::k_tab_width_ - column.value %
                IssueFormatter::k_tab_width_;
        else
            ++column.value;
    }

    return column;
}

std::string
IssueFormatterImpl::format(const std::filesystem::path source_path)
{
    std::stringstream out;
    out << make_issue_header(source_path);
    out << '\n';

    const Issue::RenderingLineSpan span = target_.compute_rendering_span(loc_tracker_);
    for (SrcLineIdx line_no = span.begin_line; line_no <= span.end_line; ++line_no)
    {
        out << format_issue_line();
        ++working_line_;
    }
    return out.str();
}

std::string
IssueFormatterImpl::format_issue_line()
{
    std::string codeline = make_codeline();
    codeline = colorize_line_comment(codeline);
    std::string underline = make_underline();
    ensure_primary_start_marked(underline);
    swap_markers(underline, IssueFormatter::Markers::highlight, IssueFormatter::Markers::underline);

    if (support::is_blank_str(codeline) &&
        support::is_blank_str(underline) &&
        !target_.suggestion.has_value())
        return {};

    const std::vector<std::string> highlight_anchors =
        make_highlight_anchors(1, IssueFormatter::Markers::highlight_stem);
    const std::vector<std::string> highlight_labels = make_highlight_labels();
    const std::vector<std::string> label_lines;

    if (target_.suggestion.has_value())
    {
        const auto insertion_line = loc_tracker_.find_last_line(target_.suggestion->insert_after);
        if (working_line_ == insertion_line)
            return format_issue_line_with_suggestion(
                codeline, underline, highlight_anchors, highlight_labels
            );
    }

    constexpr auto linebox_width = IssueFormatter::k_linebox_width_;
    std::stringstream out;
    out << FMT::format("{0:>{1}} | {2}\n", working_line_.value, linebox_width, codeline);
    if (!underline.empty())
        out << FMT::format("{0:>{1}} | {2}\n", "", linebox_width, underline);

    if (!highlight_anchors.empty())
    {
        DEBUG_SMART_ASSERT(!highlight_labels.empty() && "There are anchors -> thee must be labels");
        for (const std::string& anchor : highlight_anchors)
            out << FMT::format("{0:>{1}} | {2}\n", "", linebox_width, anchor);
        for (const std::string& label : highlight_labels)
            out << FMT::format("{0:>{1}} | {2}\n", "", linebox_width, label);
    }

    const std::string out_line = out.str();
    if (target_.suggestion.has_value()) {}

    return out.str();
}

std::string
IssueFormatterImpl::format_issue_line_with_suggestion(
    const std::string& codeline,
    const std::string& underline,
    const std::vector<std::string>& highlight_anchors,
    const std::vector<std::string>& highlight_labels)
{
    DEBUG_SMART_ASSERT(target_.suggestion.has_value() && " Shouldn't be called w/o suggestion");

    const auto suggestion_line_no = loc_tracker_.find_last_line(target_.suggestion->insert_after);
    const auto split_point = compute_visual_suggestion_indent_width(target_.suggestion.value());

    std::stringstream out;

    constexpr auto linebox_width = IssueFormatter::k_linebox_width_;

    for (const std::string& line : support::split_lines(target_.suggestion->desc))
        out << FMT::format(
            "{0:{1}} | {2:{3}}{4}{5}{6}{7}\n",
            "",                                        // {0}
            linebox_width,                             // {1}
            "",                                        // {2}
            split_point.value,                         // {3}
            IssueFormatter::Colors::suggestion_fg,     // {4}
            IssueFormatter::Tokens::insert_suggestion, // {5}
            line,                                      // {6}
            SGR_RESET                                  // {7}
        );
    out << FMT::format(
        "{0:{1}} | {2:{3}}{4}\n",
        "",                                                              // {0}
        linebox_width,                                                   // {1}
        "",                                                              // {2}
        split_point.value,                                               // {3}
        apply_sgr(IssueFormatter::Colors::suggestion_fg, "|", SGR_RESET) // {4}
    );
    out << FMT::format(
        "{0:{1}} | {2:{3}}{4}\n",
        "",                                                              // {0}
        linebox_width,                                                   // {1}
        "",                                                              // {2}
        split_point.value,                                               // {3}
        apply_sgr(IssueFormatter::Colors::suggestion_fg, "V", SGR_RESET) // {4}
    );

    out << FMT::format(
        "{0:>{1}} | {2}\n",
        suggestion_line_no.value, // {0}
        linebox_width,            // {1}
        codeline                  // {2}
    );
    out << FMT::format(
        "{0:{1}} | {2}\n",
        "",            // {0}
        linebox_width, // {1}
        underline      // {2}
    );
    for (const std::string& line : highlight_anchors)
        if (!underline.empty())
            out << FMT::format(
                "{0:{1}} | {2}\n",
                "",            // {0}
                linebox_width, // {1}
                line           // {2}
            );
    for (const std::string& line : highlight_labels)
        if (!underline.empty())
            out << FMT::format(
                "{0:{1}} | {2}\n",
                "",            // {0}
                linebox_width, // {1}
                line           // {2}
            );
    return out.str();
}

std::string
IssueFormatterImpl::make_issue_header(const std::filesystem::path source_path) const
{
    const char* const header_location_sgr = sgr(SGR_RESET COLOR_FG_ASCII_BOLD_WHITE);
    const char* const header_message_sgr = sgr(SGR_RESET COLOR_FG_ASCII_WHITE);
    const char* const issue_type_color = sgr(IssueFormatter::get_underline_color(target_.type));
    const char* const decorated_sections_sgr = sgr(SGR_RESET COLOR_FG_ASCII_BOLD_WHITE);
    const char* const reset_sgr = sgr(SGR_RESET);

    const std::string decorated_desc = decorate_sections(
        target_.desc,
        IssueFormatter::Markers::decorator,
        decorated_sections_sgr,
        header_message_sgr
    );

    std::stringstream out;
    out << header_location_sgr
        << FMT::format(
            "{0}:{1}:{2}: {3}: {4}",
            source_path.string(),                                            // {0} file
            target_.line(loc_tracker_).value,                                // {1} line
            target_.column(loc_tracker_).value,                              // {2} col
            apply_sgr(issue_type_color, to_string(target_.type), reset_sgr), // {3} issue type text
            apply_sgr(header_message_sgr, decorated_desc, reset_sgr)         // {4} description
        )
        << reset_sgr;
    return out.str();
}

std::string
IssueFormatterImpl::make_codeline()
{
    const auto crosses_on_working_line = [this](const Highlight& hl)
    {
        const SrcLineIdx first = loc_tracker_.find_first_line(hl.loc);
        const SrcLineIdx last = loc_tracker_.find_last_line(hl.loc);
        return first <= working_line_ && working_line_ <= last;
    };
    std::vector<TaggedHighlight> crossed_highlights =
        collect_line_highlight_tags(crosses_on_working_line, &sort_policy::rightmost_first);
    std::string line_accumulator;
    SrcColumnIdx column = SrcColumnIdx::none();

    const SrcBuffIdx line_start_idx = loc_tracker_.find_index_of_line(working_line_);
    for (SrcBuffIdx idx = line_start_idx; ; ++idx)
    {
        const char ch = source_buffer_[idx];
        if (ch == '\0' || ch == '\n')
            break;

        const auto slots = calculate_slots(column, ch);
        handle_possible_coloring_stop(line_accumulator, crossed_highlights, idx);
        handle_possible_coloring_start(line_accumulator, crossed_highlights, idx, false);
        line_accumulator.append(slots, ch == '\t' ? ' ' : ch);
    }
    finalize_colored_line_accumulator(line_accumulator);
    return line_accumulator;
}

std::string
IssueFormatterImpl::make_underline()
{
    const auto crosses_on_working_line = [this](const Highlight& hl)
    {
        const SrcLineIdx first = loc_tracker_.find_first_line(hl.loc);
        const SrcLineIdx last = loc_tracker_.find_last_line(hl.loc);
        return first <= working_line_ && working_line_ <= last;
    };
    std::vector<TaggedHighlight> crossed_highlights =
        collect_line_highlight_tags(crosses_on_working_line, &sort_policy::rightmost_first);

    std::string line_accumulator;
    SrcColumnIdx column = SrcColumnIdx::none();

    const SrcBuffIdx line_start_idx = loc_tracker_.find_index_of_line(working_line_);
    const SrcBuffIdx end_of_code_idx = find_end_of_code_in_line(line_start_idx); // Inclusive
    OnceFlag seen_char;
    for (SrcBuffIdx idx = line_start_idx; ; ++idx)
    {
        const char ch = source_buffer_[idx];
        const bool in_primary_issue = idx >= target_.loc.begin && idx < target_.loc.end;

        if (ch == '\0' || ch == '\n' || idx > end_of_code_idx)
        {
            if (in_primary_issue && source_blank_afterwards(idx)) // for when you reach EOF
            {
                line_accumulator += IssueFormatter::get_underline_color(target_.type);
                line_accumulator += IssueFormatter::Markers::underline;
                if (!colored_working_line_) colored_working_line_.enable();
            }
            break;
        }
        if (!std::isspace(static_cast<unsigned char>(ch)))
            seen_char.raise();

        const auto slots = calculate_slots(column, ch);
        const bool is_under_code = seen_char && idx <= end_of_code_idx;
        handle_possible_coloring_stop(line_accumulator, crossed_highlights, idx);
        handle_possible_coloring_start(line_accumulator, crossed_highlights, idx, is_under_code);

        if (coloring_highlight_ && is_under_code)
            line_accumulator.append(slots, IssueFormatter::Markers::highlight);
        else if (in_primary_issue && is_under_code)
            line_accumulator.append(slots, IssueFormatter::Markers::underline);
        else
            line_accumulator.append(slots, ' ');
    }
    // DEBUG_SMART_ASSERT(crossed_highlights.empty() && "Some highlight(s) wasn't shown correctly");
    finalize_colored_line_accumulator(line_accumulator);
    return line_accumulator;
}

std::vector<std::string>
IssueFormatterImpl::make_highlight_anchors(
    const std::size_t root_height,
    const char anchor_marker)
{
    const auto begins_on_working_line = [this](const Highlight& hl)
    {
        return loc_tracker_.find_first_line(hl.loc) == working_line_;
    };
    const std::vector<TaggedHighlight> initiating_highlights =
        collect_line_highlight_tags(begins_on_working_line, &sort_policy::leftmost_first);
    if (initiating_highlights.empty())
        return {};

    std::vector<std::string> root_lines;
    const SrcBuffIdx line_start_idx = loc_tracker_.find_index_of_line(working_line_);
    for (std::size_t h = 0; h < root_height; ++h)
    {
        SrcColumnIdx column = SrcColumnIdx::none();
        std::string line_accumulator;
        std::size_t stems_printed = 0;
        const auto required_stems = initiating_highlights.size();
        for (SrcBuffIdx line_idx = line_start_idx; ; ++line_idx)
        {
            const char ch = source_buffer_[line_idx];
            DEBUG_SMART_ASSERT(ch != '\0' && "all initiating highlights before end of buffer=");
            DEBUG_SMART_ASSERT(initiating_highlights.size() >= stems_printed);
            const TaggedHighlight& next_highlight = initiating_highlights[stems_printed];
            OnceFlag printed;
            const auto slots = calculate_slots(column, ch);
            if (is_index_on_start_of_highlight(line_idx, next_highlight))
            {
                ++stems_printed;
                printed.raise();
                line_accumulator += IssueFormatter::get_highlight_color(next_highlight.tag());
                line_accumulator += anchor_marker;
                line_accumulator += SGR_RESET;
            }
            if (stems_printed == required_stems)
                break;
            line_accumulator.append(slots - printed, ' ');
        }
        root_lines.push_back(std::move(line_accumulator));
        line_accumulator.clear();
    }
    return root_lines;
}

std::vector<std::string>
IssueFormatterImpl::make_highlight_labels()
{
    const auto begins_on_working_line = [this](const Highlight& hl)
    {
        return loc_tracker_.find_first_line(hl.loc) == working_line_;
    };
    const std::vector<TaggedHighlight> initiating_highlights =
        collect_line_highlight_tags(begins_on_working_line, &sort_policy::leftmost_first);
    if (initiating_highlights.empty())
        return {};

    std::vector<std::string> label_lines;
    const SrcBuffIdx initial_line_idx = loc_tracker_.find_index_of_line(working_line_);
    for (auto required_labels = initiating_highlights.size(); required_labels > 0; --
         required_labels)
    {
        SrcColumnIdx column = SrcColumnIdx::none();
        std::string line_accumulator;
        std::size_t labels_printed = 0;
        for (SrcBuffIdx line_idx = initial_line_idx; ; ++line_idx)
        {
            const char ch = source_buffer_[line_idx];
            DEBUG_SMART_ASSERT(ch != '\0' && "all initiating highlights before end of buffer=");
            DEBUG_SMART_ASSERT(initiating_highlights.size() > labels_printed);
            const TaggedHighlight& next_highlight = initiating_highlights[labels_printed];
            const auto slots = calculate_slots(column, ch);
            OnceFlag printed;
            if (is_index_on_start_of_highlight(line_idx, next_highlight))
            {
                ++labels_printed;
                printed.raise();
                line_accumulator += IssueFormatter::get_highlight_color(next_highlight.tag());
                if (labels_printed == required_labels)
                    line_accumulator += next_highlight.ref()->desc;
                else
                    line_accumulator += IssueFormatter::Markers::highlight_stem;
                line_accumulator += SGR_RESET;
            }
            if (labels_printed == required_labels)
                break;
            line_accumulator.append(slots - printed, ' ');
        }
        label_lines.push_back(std::move(line_accumulator));
        line_accumulator.clear();
    }

    return label_lines;
}

bool
IssueFormatterImpl::source_blank_afterwards(const SrcBuffIdx idx) const noexcept
{
    for (SrcBuffIdx i = idx; i < source_buffer_.source_size(); ++i)
        if (!std::isspace(source_buffer_[i]))
            return false;
    return true;
}

void
IssueFormatterImpl::ensure_primary_start_marked(std::string& underline)
{
    if (primary_beginning_marked_)
        return; // Already set.
    const auto first_underline_pos = underline.find_first_of(IssueFormatter::Markers::underline);
    if (first_underline_pos == std::string::npos)
        return; // Not set yet, but not underline too.
    underline[first_underline_pos] = IssueFormatter::Markers::pointer;
    primary_beginning_marked_.raise();
}

void
IssueFormatterImpl::handle_possible_coloring_start(
    std::string& line_accumulator,
    const std::vector<TaggedHighlight>& highlights,
    const SrcBuffIdx idx,
    const bool should_try_color_primary)
{
    if (!colorize_)
        return;

    const bool should_color_for_highlights =
        !highlights.empty() &&
        is_index_on_highlight(idx, highlights.back()) &&
        !coloring_highlight_;
    if (should_color_for_highlights)
    {
        line_accumulator.append(IssueFormatter::get_highlight_color(highlights.back().tag()));
        coloring_highlight_.enable();
        if (coloring_primary_) coloring_primary_.disable();
        if (!colored_working_line_) colored_working_line_.enable();
        return;
    }

    const bool in_primary_issue = idx >= target_.loc.begin && idx < target_.loc.end;
    const bool should_color_for_primary =
        !coloring_highlight_ &&
        should_try_color_primary &&
        !coloring_primary_ &&
        in_primary_issue;
    if (should_color_for_primary)
    {
        line_accumulator.append(IssueFormatter::get_underline_color(target_.type));
        coloring_primary_.enable();
        if (!colored_working_line_) colored_working_line_.enable();
        return;
    }
}

void
IssueFormatterImpl::handle_possible_coloring_stop(
    std::string& line_accumulator,
    std::vector<TaggedHighlight>& highlights,
    const SrcBuffIdx idx)
{
    if (!colorize_)
        return;

    const bool should_stop_coloring_highlight =
        !highlights.empty() &&
        highlights.back().ref()->loc.end.value == idx.value &&
        coloring_highlight_;
    if (!should_stop_coloring_highlight)
        return;
    line_accumulator += SGR_RESET;
    coloring_highlight_.disable();
    highlights.pop_back();
}

void
IssueFormatterImpl::finalize_colored_line_accumulator(std::string& line_accumulator)
{
    support::rstrip(line_accumulator); // We remove redundant suffix spaces.
    if (!colorize_)
        return;
    if (colored_working_line_)
    {
        line_accumulator += SGR_RESET;
        colored_working_line_.disable();
    }
    if (coloring_primary_)
        coloring_primary_.disable();
    if (coloring_highlight_)
        coloring_highlight_.disable();
}

SrcBuffIdx
IssueFormatterImpl::find_end_of_code_in_line(const SrcBuffIdx line_start_idx) const
{
    SrcBuffIdx last_nonspace = line_start_idx;
    std::string codeline_accumulator;
    for (SrcBuffIdx idx = line_start_idx; ; ++idx)
    {
        const char ch = source_buffer_[idx];
        if (ch == '\0' || ch == '\n')
            break;

        codeline_accumulator.push_back(ch);
        if (!std::isspace(static_cast<unsigned char>(ch)))
            last_nonspace = idx;
    }
    const auto line_comment_pos = codeline_accumulator.find(IssueFormatter::Tokens::line_comment);
    if (line_comment_pos == std::string::npos)
        return last_nonspace;

    // -1 to move 1 chars before line comment token '//'
    const auto before_comment = line_start_idx.value + line_comment_pos - 1;
    DEBUG_SMART_ASSERT(support::is_in_numeric_range<SrcBuffIdx::UnderlyingType>(before_comment));
    return SrcBuffIdx{static_cast<SrcBuffIdx::UnderlyingType>(before_comment)};
}

template <typename Predicate, typename Compare>
std::vector<TaggedHighlight>
IssueFormatterImpl::collect_line_highlight_tags(
    const Predicate should_collect,
    const Compare cmp) const
{
    static_assert(
        std::is_invocable_r_v<bool, Predicate, const Highlight&>,
        "Predicate must be callable with (const Highlight &) and return bool"
    );
    static_assert(
        std::is_invocable_r_v<bool, Compare, const TaggedHighlight&, const TaggedHighlight&>,
        "Compare must be callable as bool(const TaggedHighlight&, const TaggedHighlight&)"
    );

    std::vector<TaggedHighlight> result;
    if (!target_.highlights)
        return result;

    HighlightTag tag = 0;
    for (const Highlight& h : *target_.highlights)
    {
        if (should_collect(h))
            result.emplace_back(&h, tag);
        ++tag;
    }

    std::sort(result.begin(), result.end(), cmp);
    return result;
}

std::string
IssueFormatterImpl::decorate_sections(
    const std::string_view str,
    const char marker,
    const std::string_view sgr_section_prefix,
    const std::string_view sgr_section_suffix,
    const bool keep_decorator_marker)
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
        if (!seen_marker)
        {
            seen_marker.enable();
            if (keep_decorator_marker)
                decorated += ch;
            decorated += sgr_section_prefix;
        }
        else
        {
            seen_marker.disable();
            decorated += sgr_section_suffix;
            if (keep_decorator_marker)
                decorated += ch;
        }
    }
    return decorated;
}

std::string
IssueFormatterImpl::colorize_line_comment(const std::string_view codeline)
{
    const auto line_comment_pos = codeline.find(IssueFormatter::Tokens::line_comment);
    if (line_comment_pos == std::string::npos)
        return std::string(codeline);
    std::string colored;
    colored.reserve(
        codeline.size() + sizeof(IssueFormatter::Colors::comment_color) + sizeof(SGR_RESET)
    );
    colored += codeline.substr(0, line_comment_pos);
    colored += IssueFormatter::Colors::comment_color;
    colored += codeline.substr(line_comment_pos);
    colored += SGR_RESET;
    return colored;
}

Word
IssueFormatterImpl::calculate_slots(SrcColumnIdx& column_idx, const char ch)
{
    const Word slots =
        ch == '\t'
        ? IssueFormatter::k_tab_width_ - column_idx.value % IssueFormatter::k_tab_width_
        : 1;
    column_idx.value += slots;
    return slots;
}
} // namespace alpha

namespace
{
namespace sort_policy
{
    bool
    leftmost_first(const TaggedHighlight& a, const TaggedHighlight& b)
    {
        return a.ref()->loc.begin < b.ref()->loc.begin;
    }

    bool
    rightmost_first(const TaggedHighlight& a, const TaggedHighlight& b)
    {
        return a.ref()->loc.begin > b.ref()->loc.begin;
    }
} // namespace sort_policy

bool
is_index_on_highlight(const SrcBuffIdx idx, const TaggedHighlight& hl)
{
    return hl.ref()->loc.begin <= idx && idx < hl.ref()->loc.end; // Reminder: loc.end exclusive.
}

bool
is_index_on_start_of_highlight(const SrcBuffIdx idx, const TaggedHighlight& hl)
{
    return hl.ref()->loc.begin == idx;
}

std::string
apply_sgr(const std::string_view prefix, const std::string_view text, const std::string_view suffix)
{
    std::string out;
    out.reserve(prefix.size() + text.size() + suffix.size());
    out.append(prefix).append(text).append(suffix);
    return out;
}

void
swap_markers(std::string& str, const char old_marker, const char new_marker)
{
    std::replace(str.begin(), str.end(), old_marker, new_marker);
}

[[maybe_unused]] std::optional<HighlightTag>
find_highlight_tag_at(const std::vector<TaggedHighlight>& highlights, const SrcBuffIdx idx)
{
    DEBUG_SMART_ASSERT(
        std::is_sorted(highlights.begin(),highlights.end(), &sort_policy::leftmost_first)&&
        "Invariant violation: 'highlights' must be sorted in ascending order by loc.begin\n"
        "(per sort_policy::leftmost_first) before calling current function."
    );

    for (const TaggedHighlight& hl : highlights)
        if (is_index_on_highlight(idx, hl))
            return hl.tag();
    return std::nullopt;
}
} // namespace
