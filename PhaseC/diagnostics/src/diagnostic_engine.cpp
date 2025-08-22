#include "diagnostics/diagnostic_engine.hpp"

#include <algorithm>                // for count
#include <cstring>                  // for size_t, strchr, strlen
#include <sstream>                  // for basic_stringstream, basic_ostream
#include <stdexcept>
#include <utility>                  // for move
#include <utils/debug_tools.hpp>
#include <utils/format_adapter.hpp>

#include "core/konstants.hpp"
#include "core/source_location.hpp"  // for SourceLocation, SourceLocationTracker
#include "diagnostics/diagnostic_types.hpp"
#include "utils/misc.hpp"
#include "utils/smart_assert.h" // for DEBUG_SMART_ASSERT

namespace alpha
{
DiagnosticEngine::DiagnosticEngine(
    DiagnosticEngine::Policy &&policy,
    const std::optional<std::size_t> max_errors)
    : reporter(this),
      policy_(std::move(policy)),
      max_errors(max_errors) {}

void
DiagnosticEngine::report(Issue primary, std::list<Note> &&note_list)
{
    emit(std::move(primary), std::move(note_list));
}

void
DiagnosticEngine::report(
    const Issue::Type type,
    std::string desc,
    const SourceLocation loc,
    std::list<Note> &&note_list)
{
    emit(
        Issue(type, std::move(desc), loc),
        std::move(note_list)
    );
}

void
DiagnosticEngine::emit(
    Issue &&primary, std::list<Note> &&note_list
)
{
    if (!policy_.should_emit_diagnostic())
        return;

    diagnostics_.emplace_back(std::unique_ptr<Diagnostic>(new Diagnostic(
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
