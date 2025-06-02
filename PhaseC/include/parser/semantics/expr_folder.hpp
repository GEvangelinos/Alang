#ifndef EXPR_FOLDER_HPP
#define EXPR_FOLDER_HPP

#include <cmath>
#include "expr_maker.hpp"
#include "core/alpha_core_types.hpp"
#include "core/alpha_location.hpp"

namespace Alpha
{
class ExprFolder
{
public:
    ExprFolder(ExprMaker *expr_maker, ExprSnitch *expr_snitch);

    [[nodiscard]] const Expr *fold_arithmetic(
        IOPCode iopc, const Expr *left, const Expr *right, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_uminus(const Expr *expr, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_relational_equality(
        IOPCode iopc, const Expr *left, const Expr *right);
    [[nodiscard]] const Expr *fold_relational_arithmetic(
        IOPCode iopc, const Expr *left, const Expr *right);
    [[nodiscard]] const Expr *fold_logical_or(const Expr *left, const Expr *right);
    [[nodiscard]] const Expr *fold_logical_and(const Expr *left, const Expr *right);
    [[nodiscard]] const Expr *fold_logical_not(const Expr *expr);

private:
    ExprMaker *const expr_maker_;
    ExprSnitch *const snitch_;

    [[nodiscard]] AlphaFloat extract_alpha_float(const Expr *e);
};

inline ExprFolder::ExprFolder(ExprMaker *const expr_maker, ExprSnitch *const expr_snitch)
    : expr_maker_(Utils::require_ptr(expr_maker)),
      snitch_(Utils::require_ptr(expr_snitch)) {}

inline const Expr *ExprFolder::fold_arithmetic(
    const IOPCode iopc,
    const Expr *left,
    const Expr *right,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!left, !!right,
                       SemUtils::is_const_arithmetic_expr(left),
                       SemUtils::is_const_arithmetic_expr(right));

    const auto fold_arith_op =
        [this, iopc, result_loc](const auto l, const auto r) -> const Expr *
    {
        switch (iopc)
        {
        case IOPCode::ADD: return expr_maker_->make_const_float_expr(l + r, result_loc);
        case IOPCode::SUB: return expr_maker_->make_const_float_expr(l - r, result_loc);
        case IOPCode::MUL: return expr_maker_->make_const_float_expr(l * r, result_loc);
        case IOPCode::DIV: return expr_maker_->make_const_float_expr(l / r, result_loc);
        case IOPCode::MOD:
            if constexpr (
                std::is_same_v<decltype(l), AlphaInt> &&
                std::is_same_v<decltype(r), AlphaInt>)
                return expr_maker_->make_const_int_expr(l % r, result_loc);
            return expr_maker_->make_const_float_expr(std::fmod(l, r), result_loc);
            [[unlikely]] default: throw std::logic_error(
                ATTACH_CONTEXT("Needed arithmetic IOPC"));
        }
    };

    return left->type == Expr::Type::CONST_INT && right->type == Expr::Type::CONST_INT
           ? fold_arith_op(static_cast<const ConstIntExpr *>(left)->value,
                           static_cast<const ConstIntExpr *>(right)->value)
           : fold_arith_op(extract_alpha_float(left),
                           extract_alpha_float(right));
}

inline const Expr *
ExprFolder::fold_uminus(const Expr *const expr, const SourceLocation result_loc)
{
    switch (expr->type)
    {
    case Expr::Type::CONST_INT: return expr_maker_->make_const_int_expr(
            -static_cast<const ConstIntExpr *>(expr)->value, result_loc);
    case Expr::Type::CONST_FLOAT: return expr_maker_->make_const_float_expr(
            -static_cast<const ConstFloatExpr *>(expr)->value, result_loc);
        [[unlikely]] default: throw std::logic_error(ATTACH_CONTEXT("Needed const numeric expr."));
    }
}

inline const Expr *
ExprFolder::fold_relational_equality(
    const IOPCode iopc,
    const Expr *const left,
    const Expr *const right)

{
    DEBUG_SMART_ASSERT(!!left, !!right,
                       SemUtils::is_rvalue_expr(left),
                       SemUtils::is_rvalue_expr(right));

    const bool left_value = SemUtils::as_bool(left);
    const bool right_value = SemUtils::as_bool(right);

    if (iopc == IOPCode::IF_EQ)
        return left_value == right_value ? expr_maker_->premade_true : expr_maker_->premade_false;
    if (iopc == IOPCode::IF_NOTEQ)
        return left_value != right_value ? expr_maker_->premade_true : expr_maker_->premade_false;
    throw std::logic_error(ATTACH_CONTEXT("Needed equality IOPCode"));
}

inline const Expr *
ExprFolder::fold_relational_arithmetic(
    const IOPCode iopc,
    const Expr *const left,
    const Expr *const right)
{
    DEBUG_SMART_ASSERT(!!left, !!right,
                       SemUtils::is_const_arithmetic_expr(left),
                       SemUtils::is_const_arithmetic_expr(right));
    const auto fold_rel_op = [this, iopc](const auto l, const auto r) -> const Expr *
    {
        switch (iopc)
        {
        case IOPCode::IF_GREATER:
            return l > r ? expr_maker_->premade_true : expr_maker_->premade_false;
        case IOPCode::IF_GREATEREQ:
            return l >= r ? expr_maker_->premade_true : expr_maker_->premade_false;
        case IOPCode::IF_LESS:
            return l < r ? expr_maker_->premade_true : expr_maker_->premade_false;
        case IOPCode::IF_LESSEQ:
            return l <= r ? expr_maker_->premade_true : expr_maker_->premade_false;
            [[unlikely]] default:
            throw std::logic_error(ATTACH_CONTEXT("Needed relational arithmetic IOPC"));
        }
    };

    return left->type == Expr::Type::CONST_INT && right->type == Expr::Type::CONST_INT
           ? fold_rel_op(static_cast<const ConstIntExpr *>(left)->value,
                         static_cast<const ConstIntExpr *>(right)->value)
           : fold_rel_op(extract_alpha_float(left),
                         extract_alpha_float(right));
}

inline const Expr *ExprFolder::fold_logical_or(const Expr *const left, const Expr *const right)
{
    DEBUG_SMART_ASSERT(!!left, !!right,
                       SemUtils::is_const_bool_expr(left),
                       SemUtils::is_const_bool_expr(right));
    return static_cast<const ConstBoolExpr *>(left)->value ||
           static_cast<const ConstBoolExpr *>(right)->value
           ? expr_maker_->premade_true
           : expr_maker_->premade_false;
}

inline const Expr *ExprFolder::fold_logical_and(const Expr *const left, const Expr *const right)
{
    DEBUG_SMART_ASSERT(!!left, !!right,
                       SemUtils::is_const_bool_expr(left),
                       SemUtils::is_const_bool_expr(right));
    return static_cast<const ConstBoolExpr *>(left)->value &&
           static_cast<const ConstBoolExpr *>(right)->value
           ? expr_maker_->premade_true
           : expr_maker_->premade_false;
}

inline const Expr *ExprFolder::fold_logical_not(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr, SemUtils::is_const_bool_expr(expr));
    return static_cast<const ConstBoolExpr *>(expr)->value
           ? expr_maker_->premade_true
           : expr_maker_->premade_false;
}

inline AlphaFloat
ExprFolder::extract_alpha_float(const Expr *const e)
{
    if (e->type == Expr::Type::CONST_FLOAT)
        return static_cast<const ConstFloatExpr *>(e)->value;
    if (e->type == Expr::Type::CONST_INT)
    {
        const AlphaInt int_value = static_cast<const ConstIntExpr *>(e)->value;
        snitch_->report_if_int_to_float_loss(int_value, e->loc);
        return static_cast<AlphaFloat>(int_value);
    }
    throw std::logic_error(ATTACH_CONTEXT("Expected CONST_INT or CONST_FLOAT Expr::Type"));
}
} // namespace Alpha

#endif //EXPR_FOLDER_HPP
