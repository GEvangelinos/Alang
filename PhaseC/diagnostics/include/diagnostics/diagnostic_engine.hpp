#ifndef DIAGNOSTIC_ENGINE_HPP
#define DIAGNOSTIC_ENGINE_HPP

#include <memory>
#include <diagnostics/diagnostic_codes.hpp>
#include <diagnostics/diagnostics.hpp>
#include <core/basics.hpp>

namespace Alpha
{
    class DiagnosticEngine : private Immobile
    {
    public:

        [[nodiscard]] const std::vector<std::unique_ptr<const CTIssue> > &
        get_compile_time_issues() const noexcept { return issues_; }

        [[nodiscard]] bool contain_issues() const noexcept { return !issues_.empty(); }
        [[nodiscard]] bool contain_fatal_errors() const noexcept { return !fatals_.empty(); }
        [[nodiscard]] bool contain_errors() const noexcept { return !errors_.empty(); }
        [[nodiscard]] bool contain_warnings() const noexcept { return !warnings_.empty(); }

    private:
        std::vector<std::unique_ptr<const CTIssue> > issues_;
        std::vector<const CTIssue *> warnings_;
        std::vector<const CTIssue *> errors_;
        std::vector<const CTIssue *> fatals_;

        // Defines variadic `report()` overloads for each diagnostic code type.
        #define DEFINE_REPORT(CODE_TYPE)                                                      \
        template <typename... Args>                                                           \
        void report(const CODE_TYPE diag_code, const SourceLocation diag_loc, Args &&...args) \
        {                                                                                     \
            report_diagnostic_error_impl(                                                     \
            diag_code,                                                                        \
            diag_loc,                                                                         \
            FMT::make_format_args(std::forward<Args>(args)...));                              \
        }
        DEFINE_REPORT(WarningCodes);
        DEFINE_REPORT(ErrorCodes);
        DEFINE_REPORT(FatalCodes);
        #undef DEFINE_REPORT

        void report_impl(WarningCodes warn_code, SourceLocation warn_loc, FMT::format_args args);
        void report_impl(ErrorCodes error_code, SourceLocation error_loc, FMT::format_args args);
        void report_impl(FatalCodes fatal_code, SourceLocation fatal_loc, FMT::format_args args);
        void store(std::unique_ptr<const CTIssue> ct_issue);
    };
} // namespace Alpha

#endif // DIAGNOSTIC_ENGINE_HPP
