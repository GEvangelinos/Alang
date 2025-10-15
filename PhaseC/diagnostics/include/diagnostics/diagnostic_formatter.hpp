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
    DiagnosticFormatter(
        const std::filesystem::path &source_path,
        const LocationTracker &loc_tracker,
        const char *source_buffer);

    [[nodiscard]] std::string format_diagnostic(const Diagnostic &diagnostic, bool colorize) const;

private:

    static constexpr u8 max_shown_lines = 10;

    struct Markers
    {
        static constexpr char decorator = '`';
        static constexpr char highlight = '.';
        static constexpr char pointer = '^';
        static constexpr char suggestion = '^';
        static constexpr char underline = '~';
    };

    struct Colors
    {
        static constexpr const char *comment_color = COLOR_FG_SOFT_BROWN;
        static constexpr const char *error_fg = COLOR_FG_ASCII_BOLD_RED;
        static constexpr const char *note_fg = COLOR_FG_ASCII_BOLD_CYAN;
        static constexpr const char *warning_fg = COLOR_FG_ASCII_BOLD_MAGENTA;
    };

    struct Tokens
    {
        static constexpr const char *line_comment = "//";
    };

    static constexpr char ellipsis_block[] = "\t...\n\t...\n\t...\n";
    static constexpr u32 k_linebox_width_ = 8;
    static constexpr u32 k_tab_width_ = 8;
    const std::string source_filename_;
    const char *const source_buffer_;
    const LocationTracker &loc_tracker_;
    mutable bool underline_pointer_flag = false;

    void build_issue_header(std::stringstream &out, const Issue &issue, bool colorize) const;
    [[nodiscard]] std::string make_codeline(const Issue &issue, SrcLineIdx line_no) const;
    [[nodiscard]] std::string make_underline(const Issue &issue, SrcLineIdx line_no) const;
    [[nodiscard]] static std::string colorize_line_comment(std::string_view codeline);

    [[nodiscard]] std::vector<std::string>
    build_highlight_labels(const Issue &issue, SrcLineIdx line_no) const;
    [[nodiscard]] u32 compute_visual_suggestion_indent_width(const Suggestion &suggestion) const;
    void format_issue_line(
        std::stringstream &out, const Issue &issue, SrcLineIdx line_no, bool colorize) const;
    [[nodiscard]] std::string format_issue(const Issue &issue, bool colorize) const;

    SrcBufferIdx find_end_of_code_in_line(SrcBufferIdx line_start_idx) const;

    static void swap_markers(std::string &str, char old_marker, char new_marker);
    [[nodiscard]] static const char *get_underline_color(Issue::Type type) noexcept;
    [[nodiscard]] static const char *get_highlight_color(std::size_t highlight_index) noexcept;
    [[nodiscard]] static std::string apply_sgr(
        std::string_view prefix, std::string_view text, std::string_view suffix);
    [[nodiscard]] static std::string decorate_sections(
        std::string_view str,
        char marker,
        std::string_view sgr_section_prefix,
        std::string_view sgr_section_suffix);
};

} // namespace alpha

#endif // DIAGNOSTIC_FORMATTER_HPP
