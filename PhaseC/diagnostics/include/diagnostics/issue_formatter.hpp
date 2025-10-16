#ifndef ISSUE_FORMATTER_HPP
#define ISSUE_FORMATTER_HPP
#include <filesystem>

#include "diagnostic_types.hpp"
#include "support/cli_color.h"

namespace alpha
{
class IssueFormatter
{
    friend class IssueFormatterImpl;

public:
    IssueFormatter(
        std::filesystem::path source_path,
        const char *source_buffer,
        const LocationTracker &loc_tracker,
        bool colorize);

    std::string format(const Issue &issue);

private:
    struct Markers
    {
        static constexpr char decorator = '`';
        static constexpr char highlight = '.';
        static constexpr char pointer = '^';
        static constexpr char suggestion = '^';
        static constexpr char underline = '~';
        static constexpr char highlight_anchor = '^';
        static constexpr char highlight_stem = '|';
    };

    struct Colors
    {
        static constexpr char comment_color[] = COLOR_FG_SOFT_BROWN;
        static constexpr char error_fg[] = COLOR_FG_ASCII_BOLD_RED;
        static constexpr char note_fg[] = COLOR_FG_ASCII_BOLD_CYAN;
        static constexpr char warning_fg[] = COLOR_FG_ASCII_BOLD_MAGENTA;
        static constexpr char suggestion_fg[] = COLOR_FG_ASCII_GREEN;
    };

    struct Tokens
    {
        static constexpr const char *line_comment = "//";
    };

    static constexpr Word k_linebox_width_ = 8;
    static constexpr Word k_tab_width_ = 8;

    [[nodiscard]] static const char *get_underline_color(Issue::Type type) noexcept;
    [[nodiscard]] static const char *get_highlight_color(std::size_t highlight_index) noexcept;

    std::filesystem::path source_path_;
    const char *source_buffer_;
    const LocationTracker &loc_tracker_;
    const bool colorize_;
};
} // namespace alpha
#endif //ISSUE_FORMATTER_HPP
