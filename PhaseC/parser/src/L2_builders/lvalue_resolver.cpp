#include "parser/L2_semantic_subsystems/lvalue_resolver.hpp"

namespace alpha
{
LvalueResolver::LvalueResolver(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}
LvalueResolver::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

const Expr *
LvalueResolver::Restricted::resolve_id(const char *id_name, const SourceLocation id_loc)
{
    const Symbol *result = symbol_table_->lookup_chain(id_name, parse_ctx_->scope_handler.scope());
    if (!result) // Symbol not found, so insert it!
    {
        result = symbol_table_->insert_variable(
            id_name,
            parse_ctx_->scope_handler.scope(),
            VarSymbol::scope_to_symbol_type(parse_ctx_->scope_handler.scope()),
            parse_ctx_->space_handler.space(),
            parse_ctx_->space_handler.next_offset(),
            id_loc
        );
    }
    else if (!ensure_reachable_symbol(result, id_name, id_loc)) // Symbol found, is it reachable?
        return nullptr;
    if (result->is_variable())
        return expr_maker_->make_variable_expr(id_loc, static_cast<const VarSymbol *>(result));
    if (result->is_progfunc())
        return expr_maker_->make_prog_func_expr(id_loc, static_cast<const FuncSymbol *>(result));
    if (result->is_libfunc())
        return expr_maker_->make_lib_func_expr(id_loc, static_cast<const FuncSymbol *>(result));
    UNREACHABLE("Resolved symbol is neither a variable nor a function: unexpected symbol type");
}

const Expr *
LvalueResolver::Restricted::resolve_local_id(const char *const id_name, const SourceLocation id_loc)
{
    const Symbol *result = symbol_table_->lookup_local(id_name, parse_ctx_->scope_handler.scope());
    if (!result)
    {
        const VarSymbol *const inserted = symbol_table_->insert_variable(
            id_name,
            parse_ctx_->scope_handler.scope(),
            VarSymbol::Type::LOCAL_VARIABLE,
            parse_ctx_->space_handler.space(),
            parse_ctx_->space_handler.next_offset(),
            id_loc
        );
        return expr_maker_->make_variable_expr(id_loc, inserted);
    }
    if (result->is_variable())
        return expr_maker_->make_variable_expr(id_loc, static_cast<const VarSymbol *>(result));
    if (result->is_progfunc())
        return expr_maker_->make_prog_func_expr(id_loc, static_cast<const FuncSymbol *>(result));
    if (result->is_libfunc())
    {
        dr_->report_local_id_shadows_libfunc(id_name, id_loc);
        return nullptr;
    }
    UNREACHABLE("Unexpected symbol type");
}

const Expr *LvalueResolver::Restricted::resolve_global_id(const char *id_name, SourceLocation id_loc)
{
    const Symbol *result = symbol_table_->lookup_global(id_name);
    if (!result)
    {
        dr_->report_unresolved_global_symbol(id_name, id_loc);
        return nullptr;
    }
    if (result->is_variable())
        return expr_maker_->make_variable_expr(id_loc, static_cast<const VarSymbol *>(result));
    if (result->is_progfunc())
        return expr_maker_->make_prog_func_expr(id_loc, static_cast<const FuncSymbol *>(result));
    if (result->is_libfunc())
        return expr_maker_->make_lib_func_expr(id_loc, static_cast<const FuncSymbol *>(result));
    UNREACHABLE("Unexpected symbol type");
}

DEBUG_ALWAYS_INLINE bool // inline hint for local call-sites
LvalueResolver::Restricted::ensure_reachable_symbol(
    const Symbol *symbol,
    const char *const id_name,
    const SourceLocation id_loc)
{
    const bool reachable =
            !symbol->is_variable() ||
            symbol->scope == k_global_scope ||
            symbol->scope == parse_ctx_->func_ctx_handler.current_function_scope();
    if (reachable)
        return true;
    dr_->report_inaccessible_var_in_func(
        id_name,
        parse_ctx_->func_ctx_handler.current_function_name(),
        id_loc,
        parse_ctx_->func_ctx_handler.current_function_location(),
        symbol->loc);
    return false;
}
} // namespace alpha
