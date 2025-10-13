#ifndef DIAGNOSTIC_FORMATTER_HPP
#define DIAGNOSTIC_FORMATTER_HPP
#include <filesystem>
#include <string>

#include "diagnostic_types.hpp"
#include "core/numeric_types.hpp"

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
    static constexpr u32 k_linebox_width_ = 8;
    static constexpr u32 k_tab_width_ = 8;
    const std::string source_filename_;
    const char *const source_buffer_;
    const LocationTracker &loc_tracker_;
    mutable bool underline_pointer_flag = false;

    void build_issue_header(std::stringstream &out, const Issue &issue, bool colorize) const;
    [[nodiscard]] std::string build_codeline(u32 line_no) const;
    [[nodiscard]] std::string build_underline(const Issue &issue, u32 line_no) const;
    [[nodiscard]] static std::string colorize_highlights(
        const std::string &underline, const char *highlight_color, const char *underline_color);
    void build_highlight_labels(const Issue &issue, u32 line_no);
    [[nodiscard]] u32 compute_visual_suggestion_indent_width(const Suggestion &suggestion) const;
    void format_issue_line(
        std::stringstream &out, const Issue &issue, u32 line_no, bool colorize) const;
    [[nodiscard]] std::string format_issue(const Issue &issue, bool colorize) const;

    [[nodiscard]] std::vector<const Highlight *>
    get_highlights_of_line(const Issue &issue, u32 line_no) const;

    [[nodiscard]] static const char *get_underline_color(Issue::Type type) noexcept;
    [[nodiscard]] static std::string apply_sgr(
        std::string_view prefix, std::string_view text, std::string_view suffix);
};
} // namespace alpha

#endif // DIAGNOSTIC_FORMATTER_HPP
