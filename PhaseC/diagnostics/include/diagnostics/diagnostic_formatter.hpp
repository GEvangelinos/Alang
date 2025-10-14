#ifndef DIAGNOSTIC_FORMATTER_HPP
#define DIAGNOSTIC_FORMATTER_HPP
#include <filesystem>
#include <string>

#include "diagnostic_types.hpp"
#include "core/numeric_types.hpp"
#include "support/cli_color.h"
#include "support/misc_tools.hpp"

namespace alpha
{
// Forward declarations (to avoid includes).
struct Suggestion;
class Issue;
class LocationTracker;

class DiagnosticFormatter
{
public:
    static constexpr u32 max_shown_lines = 10;
    static constexpr char pointer_marker = '^';
    static constexpr char underline_marker = '~';
    static constexpr char highlight_marker = '.';
    DiagnosticFormatter(
        const std::filesystem::path &source_path,
        const LocationTracker &loc_tracker,
        const char *source_buffer);

    [[nodiscard]] std::string format_diagnostic(const Diagnostic &diagnostic, bool colorize) const;

private:
    struct HighlightMeta
    {
        const Highlight *highlight;
        std::size_t id;

        HighlightMeta(const Highlight *highlight, std::size_t id);
    };

    static constexpr u32 k_linebox_width_ = 8;
    static constexpr u32 k_tab_width_ = 8;
    const std::string source_filename_;
    const char *const source_buffer_;
    const LocationTracker &loc_tracker_;
    mutable bool underline_pointer_flag = false;
    static constexpr const char * comment_color = COLOR_FG_SOFT_BROWN;

    void build_issue_header(std::stringstream &out, const Issue &issue, bool colorize) const;
    [[nodiscard]] std::string build_codeline(const Issue &issue, u32 line_no) const;
    [[nodiscard]] std::string build_underline(const Issue &issue, u32 line_no) const;
    [[nodiscard, deprecated]] static std::string colorize_underline(
        const std::string &underline, const char *underline_color);
    [[nodiscard]] static std::string colorize_codeline(const std::string &codeline);
    static void swap_highlight_marker(std::string &underline);

    [[nodiscard]] std::vector<std::string>
    build_highlight_labels(const Issue &issue, u32 line_no) const;
    [[nodiscard]] u32 compute_visual_suggestion_indent_width(const Suggestion &suggestion) const;
    void format_issue_line(
        std::stringstream &out, const Issue &issue, u32 line_no, bool colorize) const;
    [[nodiscard]] std::string format_issue(const Issue &issue, bool colorize) const;

    template<typename Predicate>
    [[nodiscard]] std::vector<HighlightMeta >
    filter_highlights(const Issue &issue, Predicate pred) const;

    [[nodiscard]] static const char *get_underline_color(Issue::Type type) noexcept;
    [[nodiscard]] static const char *get_highlight_color(std::size_t highlight_index) noexcept;
    [[nodiscard]] static std::string apply_sgr(
        std::string_view prefix, std::string_view text, std::string_view suffix);
};

template<typename Predicate>
[[nodiscard]] std::vector<DiagnosticFormatter::HighlightMeta>
DiagnosticFormatter::filter_highlights(
    const Issue &issue,
    const Predicate pred) const
{
    static_assert(
        std::is_invocable_r_v<bool, Predicate, const Highlight &>,
        "Predicate must be callable with (const Highlight &) and return bool"
    );

    std::vector<HighlightMeta> line_highlights;
    if (issue.highlights.has_value())
    {
        std::size_t hl_id = 0;
        for (const Highlight &hl : *issue.highlights)
        {
            if (std::move(pred(hl)))
                line_highlights.emplace_back(&hl, hl_id);
            ++hl_id;
        }
    }
    return line_highlights;
}
} // namespace alpha

#endif // DIAGNOSTIC_FORMATTER_HPP
