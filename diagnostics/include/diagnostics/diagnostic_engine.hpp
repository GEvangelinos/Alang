#ifndef DIAGNOSTIC_ENGINE_HPP
#define DIAGNOSTIC_ENGINE_HPP

#include <functional>
#include <list>                    // for list
#include <memory>
#include <optional>
#include <core/basics.hpp>

#include "diagnostic_types.hpp"

namespace alpha
{
class DiagnosticReporter;

class DiagnosticEngine : private Immobile
{
public:
    struct Policy
    {
        std::function<bool()> should_emit_diagnostic;    // query: should DE emit this diagnostic?
        std::function<void()> notify_max_errors_reached; // notify: maximum error limit reached.
        std::function<void()> notify_fatal_error;        // notify: a fatal error occurred.
        std::function<void()> notify_hard_error;         // notify: a hard error occurred.
    };

    class ReportKey
    {
        friend class DiagnosticReporter;
        ReportKey() = default;
    };


    explicit DiagnosticEngine(Policy&& policy, std::optional<std::size_t> max_errors = std::nullopt);

    [[nodiscard]] DiagnosticReporter& reporter() { return *reporter_; }

    void report(
        ReportKey,
        DiagnosticCode code,
        Issue&& primary,
        std::list<Note>&& note_list = std::list<Note>());

    void report_syntax_error(Issue primary, std::list<Note>&& note_list = std::list<Note>());

    // TODO: add an export function for all diagnostics (export the diagnostics vector) // or DETATCH method
    [[nodiscard]] const auto& get_diagnostics() const noexcept { return diagnostics_; }
    [[nodiscard]] bool has_issues() const noexcept { return !diagnostics_.empty(); }
    [[nodiscard]] bool has_warnings() const noexcept { return !warnings_.empty(); }
    [[nodiscard]] bool has_soft_errors() const noexcept { return !softs_.empty(); }
    [[nodiscard]] bool has_hard_errors() const noexcept { return !hards_.empty(); }
    [[nodiscard]] bool has_fatal_errors() const noexcept { return !fatals_.empty(); }

    [[nodiscard]] bool has_errors() const noexcept
    {
        return has_soft_errors() || has_hard_errors() || has_fatal_errors();
    }

    [[nodiscard]] std::size_t error_count() const noexcept
    {
        return softs_.size() + hards_.size() + fatals_.size();
    }

private:
    const Policy policy_;
    const std::optional<std::size_t> max_errors;
    std::vector<std::unique_ptr<const Diagnostic>> diagnostics_;
    std::vector<const Diagnostic*> warnings_;
    std::vector<const Diagnostic*> softs_;
    std::vector<const Diagnostic*> hards_;
    std::vector<const Diagnostic*> fatals_;
    std::unique_ptr<DiagnosticReporter> reporter_;

    void emit(DiagnosticCode code, Issue&& primary, std::list<Note>&& note_list);
};
} // namespace alpha
#endif // DIAGNOSTIC_ENGINE_HPP
