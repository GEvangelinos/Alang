#include "diagnostics/diagnostic_engine.hpp"

#include <algorithm>                // for count
#include <cstring>                  // for size_t, strchr, strlen
#include <sstream>                  // for basic_stringstream, basic_ostream
#include <stdexcept>
#include <utility>                  // for move
#include <support/debug_tools.hpp>
#include <support/format_adapter.hpp>

#include <diagnostics/diagnostic_reporter.gen.hpp>
#include "core/konstants.hpp"
#include "core/source_location.hpp"  // for SourceLocation, SourceLocationTracker
#include "support/misc_tools.hpp"
#include "support/smart_assert.h" // for DMASSERT

namespace alpha
{
DiagnosticEngine::DiagnosticEngine(
    DiagnosticEngine::Policy &&policy,
    const std::optional<std::size_t> max_errors)
    : policy_(std::move(policy)),
      max_errors(max_errors),
      reporter_(std::make_unique<DiagnosticReporter>(this)) {}

void
DiagnosticEngine::report_syntax_error(Issue primary, std::list<Note> &&note_list)
{
    emit(DiagnosticCode::SYNTAX_ERROR, std::move(primary), std::move(note_list));
}

void
DiagnosticEngine::report(
    ReportKey,
    const DiagnosticCode code,
    Issue &&primary,
    std::list<Note> &&note_list)
{
    emit(code, std::move(primary),std::move(note_list));
}

void
DiagnosticEngine::emit(
    const DiagnosticCode code,
    Issue &&primary,
    std::list<Note> &&note_list)
{
    if (!policy_.should_emit_diagnostic())
        return;

    diagnostics_.emplace_back(std::unique_ptr<Diagnostic>(new Diagnostic(
        code,
        std::move(primary),
        std::move(note_list)
    )));

    switch (const Diagnostic *const d_ptr = diagnostics_.back().get(); d_ptr->primary.type)
    {
    case Issue::Type::WARNING:
        warnings_.push_back(d_ptr);
        break;
    case Issue::Type::SOFT_ERROR:
        softs_.push_back(d_ptr);
        break;
    case Issue::Type::HARD_ERROR:
        hards_.push_back(d_ptr);
        policy_.notify_hard_error();
        break;
    case Issue::Type::FATAL_ERROR:
        fatals_.push_back(d_ptr);
        policy_.notify_fatal_error();
        break;
    case Issue::Type::NOTE:
        throw std::logic_error(ATTACH_CONTEXT(
            "Issue::Type::NOTE is used to add auxiliary info. Should not be used as main Issue"));
    default: UNREACHABLE(FMT::format(
            "Unknown Issue::Type: int(type) = {}", static_cast<int>(d_ptr->primary.type)));
    }

    // We notify our policymaker, we reached maximum error limit
    if (max_errors.has_value() && error_count() >= max_errors.value())
        policy_.notify_max_errors_reached();
}
} // namespace alpha
