#ifndef LVALUE_RESOLVER_HPP
#define LVALUE_RESOLVER_HPP

#include <core/source_location.hpp>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <parser/ir.hpp>
#include <parser/parser_context.hpp>
#include <parser/symbol_table.hpp>
#include <parser/L3_ir_infra/expr_maker.hpp>
#include "L1_driver/semantic_system_support.hpp"

namespace Alpha
{
class LvalueResolver
{
public:
    explicit LvalueResolver(const SemanticSystemServices &ss_services);

    [[nodiscard ]] const Expr *resolve_id(const char *id_name, SourceLocation id_loc);

private:
    ParseCtx *const parse_ctx_;
    SymbolTable *const symbol_table_;
    DiagnosticReporter *const dr_;
    ExprMaker *const expr_maker_;

    [[nodiscard]] bool ensure_reachable_variable(
        const Symbol *symbol, const char *id_name, SourceLocation id_loc);
};

} // namespace Alpha
#endif // LVALUE_RESOLVER_HPP
