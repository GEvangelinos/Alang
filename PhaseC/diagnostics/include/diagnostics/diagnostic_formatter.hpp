#ifndef DIAGNOSTIC_PRINTER_HPP
#define DIAGNOSTIC_PRINTER_HPP
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
    struct Underline
    {
        std::string marker;
        u32 line_no;
    };

    DiagnosticFormatter(
        const std::filesystem::path &source_path,
        const LocationTracker &loc_tracker,
        const char *source_buffer);

    [[nodiscard]] std::string format_diagnostic(const Diagnostic &diagnostic, bool colorize);

private:
    const std::string source_filename_;
    const char *const source_buffer_;
    const LocationTracker &loc_tracker_;
    [[nodiscard]] std::string build_issue_header(const Issue &issue, bool colorize) const;
    [[nodiscard]] std::string build_underline(const Issue &issue, u32 line_no) const;
    [[nodiscard]] std::vector<std::string> build_suggestion_lines(const Suggestion &suggestion) const;
    [[nodiscard]] std::string format_issue(const Issue &issue, bool colorize) const ;

    [[nodiscard]] static const char *highlight_color(Issue::Type type) noexcept;
};
} // namespace alpha

#endif //DIAGNOSTIC_PRINTER_HPP
