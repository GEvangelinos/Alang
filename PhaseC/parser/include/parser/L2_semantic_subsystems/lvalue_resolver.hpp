#ifndef LVALUE_RESOLVER_HPP
#define LVALUE_RESOLVER_HPP

#include <core/source_location.hpp>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <parser/ir.hpp>
#include <parser/parser_context.hpp>
#include <parser/symbol_table.hpp>
#include <parser/L3_ir_infra/expr_maker.hpp>
#include "L1_driver/semantic_system_support.hpp"
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>

#include "semantic_subsystem.hpp"

namespace Alpha
{
class LvalueResolver
{
    friend class SemanticSystem;

private:
    class Restricted final : private SemanticSubsystem
    {
        friend class LvalueResolver;

    private:
        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        [[nodiscard]] const Expr *resolve_id(const char *id_name, SourceLocation id_loc);
        [[nodiscard]] const Expr *resolve_local_id(const char *id_name, SourceLocation id_loc);
        [[nodiscard]] const Expr *resolve_global_id(const char *id_name, SourceLocation id_loc);
        [[nodiscard]] const Expr *resolve_lvalue_to_rvalue(const Expr *lvalue);

        [[nodiscard]] bool ensure_reachable_symbol(
            const Symbol *symbol, const char *id_name, SourceLocation id_loc);
    };

    Restricted DISPATCH_TARGET;

    explicit LvalueResolver(const SemanticSystemServices &ss_services);

    DISPATCH_DECLARE_HANDLER();
};

DISPATCH_DEFINE_HANDLER_BEGIN(LvalueResolver);
    DISPATCH_BEGIN_CALLS();
    DISPATCH_SLAVE_METHOD_CALL(resolve_id);
    DISPATCH_SLAVE_METHOD_CALL(resolve_local_id);
    DISPATCH_SLAVE_METHOD_CALL(resolve_global_id);
    DISPATCH_SLAVE_METHOD_CALL(resolve_lvalue_to_rvalue);
    DISPATCH_END_CALLS();
DISPATCH_DEFINE_HANDLER_END(LvalueResolver);

inline const Expr *
LvalueResolver::Restricted::resolve_lvalue_to_rvalue(const Expr *const lvalue)
{
    return ss_bridge_->emit_tablegetelem_if_table_item(lvalue);
}
} // namespace Alpha
#endif // LVALUE_RESOLVER_HPP
