#include "L1_driver/semantic_system_support.hpp"
#include <utils/debug_tools.hpp>
#include "parser/konstants.hpp"
#include  "core/konstants.hpp"
#include "L3_ir_infra/expr_maker.hpp"

#include <parser/ir_opcode.gen.hpp>

namespace alpha
{
SemanticSystemBridge::SemanticSystemBridge(
    ParseCtx *const parse_ctx,
    ExprMaker *const expr_maker,
    QuadHandler *const quad_handler)
    : parse_ctx_(utils::require_ptr(parse_ctx)),
      expr_maker_(utils::require_ptr(expr_maker)),
      quad_handler_(utils::require_ptr(quad_handler)) {}

/// Materialize an lvalue base for further member/index access.
/// If `lvalue` is a TABLE_ITEM, emits IR (TABLEGETELEM) and returns a temp variable;
/// otherwise returns `lvalue` unchanged.
const Expr *
SemanticSystemBridge::materialize_if_table_item(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    auto *const qh = quad_handler_; // Short alias for readability.
    if (expr->type != Expr::Type::TABLE_ITEM)
        return expr;
    const auto *const ti_expr = static_cast<const TableItemExpr *>(expr);
    const auto *const temp_var = expr_maker_->make_variable_expr(expr->loc, parse_ctx_->new_temp());
    qh->emit_next(ir::Opcode::TABLEGETELEM, temp_var, ti_expr, ti_expr->index, ti_expr->loc);
    return temp_var;
}

void
SemanticSystemBridge::finalize_bool_expr(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::BOOL_EXPR)
        return; // Nothing to backpatch if not bool_expr.

    const BoolExpr *const bool_expr = static_cast<const BoolExpr *>(expr);
    auto *const qh = quad_handler_; // Short alias for readability.

    DEBUG_SMART_ASSERT(!!bool_expr->var_symbol);

    // true branch: patch to here and assign true
    qh->patch_list(bool_expr->true_list, qh->next_quad_label());
    qh->emit_next(ir::Opcode::ASSIGN, expr, &k_static_true_expr, nullptr, expr->loc);

    // Offset to land after the false branch
    constexpr LabelID past_false_branch_offset = 2; // Depends on how many emits occur after jump.
    qh->emit_next(ir::Opcode::JUMP, nullptr, nullptr, nullptr, expr->loc, past_false_branch_offset);

    // false branch: patch to here and assign false
    qh->patch_list(bool_expr->false_list, qh->next_quad_label());
    qh->emit_next(ir::Opcode::ASSIGN, expr, &k_static_false_expr, nullptr, expr->loc);
}
} // namespace alpha
