#ifndef LVALUE_RESOLVER_HPP
#define LVALUE_RESOLVER_HPP

#include <core/source_location.hpp>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <parser/ir.hpp>
#include <parser/parser_context.hpp>
#include <parser/symbol_table.hpp>
#include <parser/L3_ir_infra/expr_maker.hpp>

namespace Alpha
{
class LvalueResolver
{
public:
    explicit LvalueResolver(const SemanticSystemServices &init_pack);

    const Expr *resolve_id(const char *id_name, SourceLocation id_loc);

private:
    ParseCtx *const parse_ctx_;
    SymbolTable *const symbol_table_;
    DiagnosticReporter *const dr_;
    ExprMaker *const expr_maker_;
};

inline LvalueResolver::LvalueResolver(const SemanticSystemServices &init_pack)
    : parse_ctx_(init_pack.parse_ctx),
      symbol_table_(init_pack.symbol_table),
      dr_(init_pack.dr),
      expr_maker_(init_pack.expr_maker) {}

inline const Expr *
LvalueResolver::resolve_id(const char *id_name, const SourceLocation id_loc)
{
    const Symbol *symbol = symbol_table_->lookup_chain(id_name, parse_ctx_->scope_handler.scope());
    if (!symbol)
    {
        const Variable::Type var_type = parse_ctx_->scope_handler.scope() == k_global_scope
                                        ? Variable::Type::GLOBAL_VARIABLE
                                        : Variable::Type::LOCAL_VARIABLE;
        symbol = symbol_table_->insert_variable(
            id_name,
            parse_ctx_->scope_handler.scope(),
            var_type,
            parse_ctx_->space_handler.space(),
            parse_ctx_->space_handler.next_offset(),
            id_loc);
    }
    else if (symbol->is_variable() &&
             symbol->scope > k_global_scope &&
             symbol->scope <= parse_ctx_->function_ctx_handler.current_function_scope())
    {
        dr_->report_inaccessible_variable_in_func(
            id_name,
            parse_ctx_->function_ctx_handler.current_function_name(),
            id_loc,
            parse_ctx_->function_ctx_handler.current_function_location(),
            symbol->loc);
    }

    return expr_maker_->make_variable_expr(symbol, symbol->loc);
}
} // namespace Alpha
#endif // LVALUE_RESOLVER_HPP
