#include "L1_driver/semantic_driver_services.hpp"

namespace Alpha{


SemanticDriverServices::SemanticDriverServices(
ParseCtx *const parse_ctx,
 ExprMaker *const expr_maker,
 QuadHandler *const quad_handler)
: parse_ctx_(parse_ctx), expr_maker_(expr_maker), quad_handler_(quad_handler) {}


const Expr *
SemanticDriverServices::emit_quad_if_table_item(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::TABLE_ITEM)
        return expr;
    const auto *const ti_expr = static_cast<const TableItemExpr *>(expr);
    const auto *const temp_var = expr_maker_->make_variable_expr(parse_ctx_->new_temp(), k_no_loc);
    quad_handler_->emit_next_quad(
        IOPCode::TABLEGETELEM, ti_expr, ti_expr->index, temp_var, ti_expr->loc);
    return temp_var;
}
} // namespace Alpha
