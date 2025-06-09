#include <stdexcept>
#include <diagnostics/diagnostic_engine.hpp>
#include <utils/debug_tools.hpp>
#include <utils/format_adapter.hpp>

namespace Alpha
{
    void DiagnosticEngine::report_impl(
        const ErrorCodes error_code,
        const SourceLocation error_loc,
        const FMT::format_args args)
    {
        switch (error_code)
        {
            // TODO: purge this,, and integrate with diagnostic generator
            case ErrorCodes::ASSIGNMENT_LHS_IS_FUNC:
            {
                const std::string error = FMT::vformat(
                    "lvalue required as left operand of assignment."
                    " Left operand's expression type is `{0}` ",
                    args);
                store(std::unique_ptr<const CTIssue>(
                        new const CTIssue(Issue::Type::ERROR, error, error_loc)
                    )
                );
                break;
            }
            default: throw std::logic_error(ATTACH_CONTEXT("Unknown diagnostic error code."));
        }
    }

    void DiagnosticEngine::store(std::unique_ptr<const CTIssue> ct_issue)
    {
        switch (const auto raw_ptr = ct_issue.get(); raw_ptr->issue.type)
        {
            case Issue::Type::WARNING:
                warnings_.push_back(raw_ptr);
                break;
            case Issue::Type::ERROR:
                errors_.push_back(raw_ptr);
                break;
            case Issue::Type::FATAL_ERROR:
                fatals_.push_back(raw_ptr);
                break;
            case Issue::Type::NOTE:
                throw std::logic_error(ATTACH_CONTEXT(
                    "Issue::Type::NOTE is used to add auxiliary info. Should not be used as main Issue"));
            default: UNREACHABLE("Unknown Issue::Type.");
        }
        issues_.push_back(std::move(ct_issue)); // Pass ownership to `issues_` vector.
    }
} // namespace Alpha
