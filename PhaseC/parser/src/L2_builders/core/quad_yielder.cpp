#include "L2_semantic_subsystems/core/quad_yielder.hpp"

namespace alpha
{
QuadYielder::QuadYielder(
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    ExprMaker *const expr_maker,
    QuadHandler *const quad_handler,
    QuadInterceptor *const quad_interceptor)
    : parse_ctx_(support::require_ptr(parse_ctx)),
      symbol_table_(support::require_ptr(symbol_table)),
      expr_maker_(support::require_ptr(expr_maker)),
      quad_handler_(support::require_ptr(quad_handler)),
      quad_interceptor_(support::require_ptr(quad_interceptor)) {}

void
QuadYielder::release_temp_handle_if_active(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    // Rvalue operands don't persist, so temp names can be safely reused.
    if (expr->has_active_temp())
    {
        const VarSymbol *const var_symbol = static_cast<const ExprWVarSymbol *>(expr)->var_symbol;
        parse_ctx_->temp_ctx_handler.release_temp_handle(var_symbol->temp_handle());
        symbol_table_->detach_temp_handle(var_symbol);
    }
}

const Expr *
QuadYielder::yield(
    const ir::Opcode opc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc,
    const LabelID label)
{
    return yield(opc, [result]() { return result; }, arg1, arg2, loc, label);
}

const Expr *
QuadYielder::yield_next(
    const ir::Opcode opc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc,
    const LabelID label_offset)
{
    return yield_next(opc, [result]() { return result; }, arg1, arg2, loc, label_offset);
}

const Expr *
QuadYielder::yield_labelless(
    const ir::Opcode opc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc)
{
    return yield_labelless(opc, [result]() { return result; }, arg1, arg2, loc);
}
} // namespace alpha
