#ifndef LVALUE_RESOLVER_HPP
#define LVALUE_RESOLVER_HPP

#include <core/source_location.hpp>
#include <parser/parser_context.hpp>
#include <parser/symbol_table.hpp>
#include "L2_semantic_subsystems/core/expr_maker.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>

#include "semantic_subsystem.hpp"
#include "core/string_span.hpp"

namespace alpha
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

        [[nodiscard]] const Expr *resolve_id(StringSpan id_name, SourceLocation id_loc);
        [[nodiscard]] const Expr *resolve_local_id(StringSpan lid_name, SourceLocation lid_loc);
        [[nodiscard]] const Expr *resolve_global_id(StringSpan gid_name, SourceLocation gid_loc);
        [[nodiscard]] const Expr *resolve_lvalue_to_rvalue(const Expr *lvalue);

        [[nodiscard]] bool ensure_reachable_symbol(
            const Symbol *symbol, StringSpan id_name, SourceLocation id_loc);
    };

    Restricted DISPATCH_TARGET;

    explicit LvalueResolver(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_SLAVE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(resolve_id);
    DISPATCH_SLAVE_METHOD_CALL(resolve_local_id);
    DISPATCH_SLAVE_METHOD_CALL(resolve_global_id);
    DISPATCH_SLAVE_METHOD_CALL(resolve_lvalue_to_rvalue);
    DISPATCH_DEFINE_HANDLER_END();
};

} // namespace alpha
#endif // LVALUE_RESOLVER_HPP
