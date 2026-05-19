#include "ir_postprocess/ir_validator.hpp"
#include <unordered_map>
#include "core/ir/ir_expr.hpp"
#include "diagnostics/diagnostic_reporter.gen.hpp"
#include "core/ir/ir_quad.hpp"
#include "settings/compiler_settings.hpp"

namespace alpha
{
IRValidator::IRValidator(const settings::ConfigFlags& config_flags, DiagnosticReporter& dr)
    : config_flags_(config_flags),
      dr_(dr) {}

void
IRValidator::check_uninitialized_reads(const ir::QuadStream& quads)
{
    struct UninitTrace
    {
        const SourceLocation first_usage_loc;
        u32 count = 1;
    };
    std::unordered_map<const VarSymbol*, UninitTrace> uninitialized_vars;
    std::vector<const VarSymbol*> culprits;


    const auto check_and_report = [&uninitialized_vars, &culprits](const Expr* const arg)
    {
        if (arg && arg->has_uninitialized_variable())
        {
            DMASSERT(arg->has_var_symbol());
            const auto* const var_symbol = static_cast<const ExprWVarSymbol*>(arg)->var_symbol;
            if (uninitialized_vars.contains(var_symbol))
                ++uninitialized_vars.at(var_symbol).count;
            else
            {
                uninitialized_vars.emplace(var_symbol, UninitTrace{.first_usage_loc = arg->loc});
                culprits.push_back(var_symbol);
            }
        }
    };

    for (const auto& q : quads)
    {
        check_and_report(q.arg1);
        check_and_report(q.arg2);
    }

    for (const VarSymbol* const var : culprits)
    {
        const UninitTrace trace = uninitialized_vars[var];
        const auto tally = trace.count > 1 ? FMT::format(" (1 of {})", trace.count) : std::string{};
        dr_.report_uninitialized_rvalue(var->name, tally, trace.first_usage_loc, var->loc);
    }
}

void
IRValidator::run(const ir::QuadStream& quads)
{
    if (config_flags_.extra_warnings)
        check_uninitialized_reads(quads);
}
} // namespace alpha
