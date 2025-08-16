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
    :       parse_ctx_(REQUIRE_PTR(parse_ctx)),
      expr_maker_(REQUIRE_PTR(expr_maker)),
      quad_handler_(REQUIRE_PTR(quad_handler)) {}

/// Materialize an lvalue base for further member/index access.
/// If `lvalue` is a TABLE_ITEM, emits IR (TABLEGETELEM) and returns a temp variable;
/// otherwise returns `lvalue` unchanged.
/// @note Deprecated name: emit_if_table_item (kept below as a wrapper for migration).
const Expr *
SemanticSystemBridge::materialize_if_table_item(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr,);
    if (expr->type != Expr::Type::TABLE_ITEM)
        return expr;
    const auto *const ti_expr = static_cast<const TableItemExpr *>(expr);
    const auto *const temp_var = expr_maker_->make_variable_expr(
        expr->loc, parse_ctx_->new_temp());
    quad_handler_->emit_next(
        ir::Opcode::TABLEGETELEM, temp_var, ti_expr, ti_expr->index, ti_expr->loc);
    return temp_var;
}
} // namespace alpha
