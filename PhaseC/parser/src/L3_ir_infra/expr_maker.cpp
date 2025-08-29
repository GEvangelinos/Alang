#include "L3_ir_infra/expr_maker.hpp"

namespace alpha
{
ExprMaker::ExprMaker(ParseCtx *const parse_ctx)
    : parse_ctx_(utils::require_ptr(parse_ctx)) {}

ExprMaker::~ExprMaker() noexcept
{
    for (const Expr *e: expr_sink_)
    {
        DEBUG_SMART_ASSERT(!!e);
        using ET = Expr::Type;
        switch (e->type)
        {
            // clang-format off
        case ET::ARITHMETIC_EXPR: delete static_cast<const ArithmeticExpr *>(e); break;
        case ET::ASSIGN_EXPR: delete static_cast<const AssignExpr *>(e);         break;
        case ET::BOOL_EXPR: delete static_cast<const BoolExpr *>(e);             break;
        case ET::CONST_BOOL: delete static_cast<const ConstBoolExpr *>(e);       break;
        case ET::CONST_INT: delete static_cast<const ConstIntExpr *>(e);         break;
        case ET::CONST_FLOAT: delete static_cast<const ConstFloatExpr *>(e);     break;
        case ET::CONST_STRING: delete static_cast<const ConstStringExpr *>(e);   break;
        case ET::CONST_NIL: delete static_cast<const ConstNilExpr *>(e);         break;
        case ET::LIBRARY_FUNCTION: delete static_cast<const LibFuncExpr *>(e);   break;
        case ET::PROGRAM_FUNCTION: delete static_cast<const ProgFuncExpr *>(e);  break;
        case ET::NEW_TABLE: delete static_cast<const NewTableExpr *>(e);         break;
        case ET::TABLE_ITEM: delete static_cast<const TableItemExpr *>(e);       break;
        case ET::VARIABLE: delete static_cast<const VariableExpr *>(e);          break;
        // clang-format on
        default: UNREACHABLE(FMT::format(
                "Unknown Expr::Type. Expr::Type's int value = {}", static_cast<int>(e->type)));
        }
    }
}

const Expr *
ExprMaker::clone_with_updated_location(const SourceLocation new_loc, const Expr *const donor_expr)
{
    DEBUG_SMART_ASSERT(!!donor_expr);
    using ET = Expr::Type;
    switch (donor_expr->type)
    {
    case ET::ARITHMETIC_EXPR:
        return make_arithmetic_expr<true>(new_loc, static_cast<const ArithmeticExpr *>(donor_expr));
    case ET::ASSIGN_EXPR:
        return make_assign_expr(new_loc, static_cast<const AssignExpr *>(donor_expr)->var_symbol);
    case ET::BOOL_EXPR:
    {
        // TODO: test this case... the trim_logical functions.. is it safe to move() the vectors?
        const auto *const bool_donor_expr = static_cast<const BoolExpr *>(donor_expr);
        const BoolExpr *result = make_bool_expr<true>(new_loc, bool_donor_expr);
        result->true_list = std::move(bool_donor_expr->true_list);
        result->false_list = std::move(bool_donor_expr->false_list);
        bool_donor_expr->true_list.clear();
        bool_donor_expr->false_list.clear();
        return result;
    }
    case ET::CONST_BOOL:
        return make_const_bool_expr(new_loc, static_cast<const ConstBoolExpr *>(donor_expr)->value);
    case ET::CONST_INT:
        return make_const_int_expr(new_loc, static_cast<const ConstIntExpr *>(donor_expr)->value);
    case ET::CONST_FLOAT:
        return make_const_float_expr(
            new_loc, static_cast<const ConstFloatExpr *>(donor_expr)->value);
    case ET::CONST_STRING:
        return make_const_string_expr(
            new_loc, static_cast<const ConstStringExpr *>(donor_expr)->value);
    case ET::CONST_NIL:
        return make_nil_expr(new_loc);
    case ET::LIBRARY_FUNCTION:
        return make_lib_func_expr(
            new_loc, static_cast<const LibFuncExpr *>(donor_expr)->func_symbol);
    case ET::PROGRAM_FUNCTION:
        return make_prog_func_expr(
            new_loc, static_cast<const ProgFuncExpr *>(donor_expr)->func_symbol);
    case ET::NEW_TABLE:
        return make_new_table_expr<true>(new_loc, static_cast<const NewTableExpr *>(donor_expr));
    case ET::TABLE_ITEM:
    {
        const Expr *const index = static_cast<const TableItemExpr *>(donor_expr)->index;
        return make_table_item_expr(new_loc, donor_expr, index);
    }
    case ET::VARIABLE:
        return make_variable_expr(
            new_loc, static_cast<const VariableExpr *>(donor_expr)->var_symbol);
    }
    UNREACHABLE(FMT::format(
        "Unknown Expr::Type. Expr::Type's int value = {}", static_cast<int>(donor_expr->type)));
}
} // namespace alpha
