#ifndef DIAGNOSTIC_FORMATTER_HPP
#define DIAGNOSTIC_FORMATTER_HPP
#include <filesystem>
#include <string>

#include "diagnostic_types.hpp"

namespace alpha
{
class TranslationUnitBuffer;
}

namespace alpha
{
// Forward declarations (to avoid includes).
struct Suggestion;
class Issue;
class LocationTracker;
class IssueFormatter;

class DiagnosticFormatter
{
public:
    DiagnosticFormatter(
        const std::filesystem::path &source_path,
        const LocationTracker &loc_tracker,
        const TranslationUnitBuffer &source_buffer,
        bool colorize);

    ~DiagnosticFormatter();

    [[nodiscard]] std::string format(const Diagnostic &diagnostic) const;

private:
    std::unique_ptr<IssueFormatter> issue_formatter_;
};
} // namespace alpha
#endif // DIAGNOSTIC_FORMATTER_HPP
