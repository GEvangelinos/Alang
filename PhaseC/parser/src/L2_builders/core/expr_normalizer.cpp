#include "L2_semantic_subsystems/core/expr_normalizer.hpp"
#include "ir_expr.hpp"
#include "L2_semantic_subsystems/core/quad_emitter.hpp"
#include "L2_semantic_subsystems/core/expr_maker.hpp"
#include "L2_semantic_subsystems/core/quad_yielder.hpp"

namespace alpha
{
ExprNormalizer::ExprNormalizer(
    ParseCtx *const parse_ctx,
    ExprMaker *const expr_maker,
    QuadEmitter *const quad_emitter)
    : parse_ctx_(support::require_ptr(parse_ctx)),
      expr_maker_(support::require_ptr(expr_maker)),
      quad_emitter_(support::require_ptr(quad_emitter)) {}

const Expr *
ExprNormalizer::materialize_if_table_item(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::TABLE_ITEM)
        return expr;
    const auto *const ti_expr = static_cast<const TableItemExpr *>(expr);
    const auto *const temp_var = expr_maker_->make_variable_expr(expr->loc, parse_ctx_->new_temp());
    quad_emitter_->emit(
        ir::Opcode::TABLEGETELEM,
        temp_var,
        ti_expr,
        ti_expr->index,
        ti_expr->loc,
        quad_emitter_->next_quad_label(),
        QuadEmitter::EmitterKey{}
    );
    return temp_var;
}

void
ExprNormalizer::resolve_bool_short_circuit(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::BOOL)
        return; // Nothing to backpatch if not bool_expr.

    const BoolExpr *const bool_expr = static_cast<const BoolExpr *>(expr);
    auto *const qe = quad_emitter_; // Short alias for readability.

    DEBUG_SMART_ASSERT(!!bool_expr->var_symbol);

    // true branch: patch to here and assign true
    qe->labelPatch_list(bool_expr->true_list, qe->next_quad_label());
    qe->emit(
        ir::Opcode::ASSIGN,
        expr,
        &k_static_true_expr,
        nullptr,
        expr->loc,
        qe->next_quad_label(),
        QuadEmitter::EmitterKey{}
    );

    // Offset to land after the false branch
    constexpr LabelID past_false_branch_offset = 2; // Depends on how many emits occur after jump.
    qe->emit(
        ir::Opcode::JUMP,
        nullptr,
        nullptr,
        nullptr,
        expr->loc,
        qe->next_quad_label() + past_false_branch_offset,
        QuadEmitter::EmitterKey{}
    );

    // false branch: patch to here and assign false
    qe->labelPatch_list(bool_expr->false_list, qe->next_quad_label());
    qe->emit(
        ir::Opcode::ASSIGN,
        expr,
        &k_static_false_expr,
        nullptr,
        expr->loc,
        qe->next_quad_label(),
        QuadEmitter::EmitterKey{}
    );
}
} // namespace alpha
