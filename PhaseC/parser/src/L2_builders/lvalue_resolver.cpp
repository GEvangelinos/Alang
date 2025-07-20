#include "L2_builders/lvalue_resolver.hpp"

namespace Alpha
{
LvalueResolver::LvalueResolver(const SemanticSystemServices &ss_services)
    : parse_ctx_(ss_services.parse_ctx),
      symbol_table_(ss_services.symbol_table),
      dr_(ss_services.dr),
      expr_maker_(ss_services.expr_maker) {}

const Expr *
LvalueResolver::resolve_id(const char *id_name, const SourceLocation id_loc)
{
    const Symbol *symbol = symbol_table_->lookup_chain(id_name, parse_ctx_->scope_handler.scope());
    if (!symbol) // Symbol not found, so insert it!
    {
        symbol = symbol_table_->insert_variable(
            id_name,
            parse_ctx_->scope_handler.scope(),
            VarSymbol::scope_to_symbol_type(parse_ctx_->scope_handler.scope()),
            parse_ctx_->space_handler.space(),
            parse_ctx_->space_handler.next_offset(),
            id_loc
        );
    }
    else if (!ensure_reachable_variable(symbol, id_name, id_loc))
        return nullptr;
    if (symbol->is_variable())
        return expr_maker_->make_variable_expr(id_loc, static_cast<const VarSymbol *>(symbol));
    if (symbol->is_library_function())
        return expr_maker_->make_lib_func_expr(id_loc, static_cast<const FuncSymbol *>(symbol));
    if (symbol->is_program_function())
        return expr_maker_->make_prog_func_expr(id_loc, static_cast<const FuncSymbol *>(symbol));
    UNREACHABLE("We either inserted or found a symbol, and it should be either a var or a func");
}

DEBUG_ALWAYS_INLINE bool // inline hint for local call-sites
LvalueResolver::ensure_reachable_variable(
    const Symbol *symbol,
    const char *const id_name,
    const SourceLocation id_loc)
{
    const bool unreachable_condition =
            symbol->is_variable() &&
            symbol->scope > k_global_scope &&
            symbol->scope <= parse_ctx_->func_ctx_handler.current_function_scope();
    if (!unreachable_condition)
        return true;
    dr_->report_inaccessible_variable_in_func(
        id_name,
        parse_ctx_->func_ctx_handler.current_function_name(),
        id_loc,
        parse_ctx_->func_ctx_handler.current_function_location(),
        symbol->loc);
    return false;
}
} // namespace Alpha
