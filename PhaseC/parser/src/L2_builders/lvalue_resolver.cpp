#include "L2_semantic_subsystems/lvalue_resolver.hpp"
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include "L2_semantic_subsystems/core/expr_normalizer.hpp"

namespace alpha
{
LvalueResolver::LvalueResolver(const SemanticSystemServices& ss_services)
    : DISPATCH_TARGET(ss_services) {}

LvalueResolver::Restricted::Restricted(const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services) {}

const Expr*
LvalueResolver::Restricted::resolve_id(const StringSpan id_name, const SourceLocation id_loc)
{
    DMASSERT(!id_name.empty());
    const Symbol* result = symbol_table_->
        lookup_nearest(id_name, parse_ctx_->scope_handler.scope());
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
        return expr_maker_->make_variable_expr(id_loc, static_cast<const VarSymbol*>(result));
    if (result->type == Symbol::Type::PROGRAM_FUNCTION)
        return expr_maker_->
            make_prog_func_expr(id_loc, static_cast<const ProgFuncSymbol*>(result));
    if (result->type == Symbol::Type::LIBRARY_FUNCTION)
        return expr_maker_->make_lib_func_expr(id_loc, static_cast<const LibFuncSymbol*>(result));
    UNREACHABLE("Resolved symbol is neither a variable nor a function: unexpected symbol type");
}

const Expr*
LvalueResolver::Restricted::resolve_local_id(
    const StringSpan lid_name,
    const SourceLocation lid_loc)
{
    DMASSERT(!lid_name.empty());

    const auto current_scope = parse_ctx_->scope_handler.scope();
    if (current_scope != k_libfunc_scope && symbol_table_->is_libfunc_name(lid_name))
    {
        dr_->report_local_id_shadows_libfunc(lid_name, lid_loc);
        return nullptr;
    }
    const Symbol* const result =
        symbol_table_->lookup_local(lid_name, parse_ctx_->scope_handler.scope());
    if (!result)
    {
        const VarSymbol* const inserted = symbol_table_->insert_variable(
            lid_name,
            current_scope,
            current_scope == k_global_scope
            ? VarSymbol::Type::GLOBAL_VARIABLE
            : VarSymbol::Type::LOCAL_VARIABLE,
            parse_ctx_->space_handler.space(),
            parse_ctx_->space_handler.next_offset(),
            lid_loc
        );
        return expr_maker_->make_variable_expr(lid_loc, inserted);
    }
    if (result->is_variable())
        return expr_maker_->make_variable_expr(lid_loc, static_cast<const VarSymbol*>(result));
    if (result->type == Symbol::Type::PROGRAM_FUNCTION)
        return expr_maker_->make_prog_func_expr(
            lid_loc, static_cast<const ProgFuncSymbol*>(result));
    DMASSERT(result->type == Symbol::Type::LIBRARY_FUNCTION);
    return expr_maker_->make_lib_func_expr(
        SourceLocation::none(), static_cast<const LibFuncSymbol*>(result));
    UNREACHABLE("Unexpected symbol type");
}

const Expr* LvalueResolver::Restricted::resolve_global_id(
    const StringSpan gid_name,
    const SourceLocation gid_loc)
{
    DMASSERT(!gid_name.empty());
    const Symbol* const result = symbol_table_->lookup_global(gid_name);
    if (!result)
    {
        dr_->report_unresolved_global_symbol(gid_name, gid_loc);
        return nullptr;
    }
    if (result->is_variable())
        return expr_maker_->make_variable_expr(gid_loc, static_cast<const VarSymbol*>(result));
    if (result->type == Symbol::Type::PROGRAM_FUNCTION)
        return expr_maker_->make_prog_func_expr(
            gid_loc, static_cast<const ProgFuncSymbol*>(result));
    if (result->type == Symbol::Type::LIBRARY_FUNCTION)
        return expr_maker_->make_lib_func_expr(gid_loc, static_cast<const LibFuncSymbol*>(result));
    UNREACHABLE("Unexpected symbol type");
}

const Expr*
LvalueResolver::Restricted::resolve_lvalue_to_rvalue(const Expr* const lvalue)
{
    DMASSERT(!!lvalue);
    return expr_normalizer_->materialize_if_table_item(lvalue);
}

bool
LvalueResolver::Restricted::ensure_reachable_symbol(
    const Symbol* symbol,
    const StringSpan id_name,
    const SourceLocation id_loc)
{
    const bool reachable =
        !symbol->is_variable() ||
        symbol->scope == k_global_scope ||
        symbol->scope > parse_ctx_->func_ctx_handler.current_function_scope();
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
