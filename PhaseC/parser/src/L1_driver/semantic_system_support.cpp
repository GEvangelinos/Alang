#include "L1_driver/semantic_system_support.hpp"
#include <utils/debug_tools.hpp>
#include "parser/konstants.hpp"
#include  "core/konstants.hpp"
#include "L3_ir_infra/expr_maker.hpp"

#include <parser/ir_opcode.hpp>

namespace alpha
{
SemanticSystemBridge::SemanticSystemBridge(
    ParseCtx *const parse_ctx,
    ExprMaker *const expr_maker,
    QuadHandler *const quad_handler)
    : parse_ctx_(REQUIRE_PTR(parse_ctx)),
      expr_maker_(REQUIRE_PTR(expr_maker)),
      quad_handler_(REQUIRE_PTR(quad_handler)) {}


const Expr *
SemanticSystemBridge::emit_tablegetelem_if_table_item(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::TABLE_ITEM)
        return expr;
    const auto *const ti_expr = static_cast<const TableItemExpr *>(expr);
    const auto *const temp_var = expr_maker_->make_variable_expr(expr->loc, parse_ctx_->new_temp());
    quad_handler_->emit_next(
        ir::Opcode::TABLEGETELEM, temp_var, ti_expr, ti_expr->index, ti_expr->loc);
    return temp_var;
}
} // namespace alpha
