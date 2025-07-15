#ifndef EXPR_FOLDER_HPP
#define EXPR_FOLDER_HPP

#include <cmath>
#include "expr_maker.hpp"
#include "parser/ir.hpp"
#include "core/source_location.hpp"

namespace Alpha
{
class ExprFolder
{
public:
    ExprFolder(ExprMaker *expr_maker, ExprSnitch *expr_snitch);

    [[nodiscard]] const Expr *fold_uminus(const Expr *expr, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_arithmetic(
        IOPCode iopc, const Expr *left, const Expr *right, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_relational_equality(
        IOPCode iopc, const Expr *left, const Expr *right, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_relational_arithmetic(
        IOPCode iopc, const Expr *left, const Expr *right, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_logical_or(
        const Expr *left, const Expr *right, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_logical_and(
        const Expr *left, const Expr *right, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_logical_not(const Expr *expr, SourceLocation result_loc);

private:
    ExprMaker *const expr_maker_;
    ExprSnitch *const snitch_;

    [[nodiscard]] AlphaFloat extract_alpha_float(const Expr *e);
};

inline ExprFolder::ExprFolder(ExprMaker *const expr_maker, ExprSnitch *const expr_snitch)
    : expr_maker_(Utils::require_ptr(expr_maker)),
      snitch_(Utils::require_ptr(expr_snitch)) {}

inline const Expr *
ExprFolder::fold_uminus(const Expr *const expr, const SourceLocation result_loc)
{
    switch (expr->type)
    {
    case Expr::Type::CONST_INT:
        return expr_maker_->make_const_int_expr(
            result_loc, -static_cast<const ConstIntExpr *>(expr)->value);
    case Expr::Type::CONST_FLOAT:
        return expr_maker_->make_const_float_expr(
            result_loc, -static_cast<const ConstFloatExpr *>(expr)->value);
    default: throw std::logic_error(ATTACH_CONTEXT("Needed const numeric expr."));
    }
}

inline const Expr *ExprFolder::fold_arithmetic(
    const IOPCode iopc,
    const Expr *left,
    const Expr *right,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(
        !!left, !!right,
        SemUtils::is_const_arithmetic_expr(left),
        SemUtils::is_const_arithmetic_expr(right)
    );

    const auto fold_arith_op =
            [this, iopc, result_loc](const auto l, const auto r) -> const Expr *
    {
        switch (iopc)
        {
        case IOPCode::ADD: return expr_maker_->make_const_float_expr(result_loc, l + r);
        case IOPCode::SUB: return expr_maker_->make_const_float_expr(result_loc, l - r);
        case IOPCode::MUL: return expr_maker_->make_const_float_expr(result_loc, l * r);
        case IOPCode::DIV: return expr_maker_->make_const_float_expr(result_loc, l / r);
        case IOPCode::MOD:
            if constexpr (std::is_same_v<decltype(l), AlphaInt> && std::is_same_v<decltype(r), AlphaInt>) // NOLINT
                return expr_maker_->make_const_int_expr(result_loc, l % r);
            return expr_maker_->make_const_float_expr(result_loc, std::fmod(l, r));
        default: throw std::logic_error(ATTACH_CONTEXT("(Needed arithmetic IOPC"));
        }
    };

    return left->type == Expr::Type::CONST_INT && right->type == Expr::Type::CONST_INT
           ? fold_arith_op(static_cast<const ConstIntExpr *>(left)->value,
                           static_cast<const ConstIntExpr *>(right)->value)
           : fold_arith_op(extract_alpha_float(left), extract_alpha_float(right));
}

inline const Expr *
ExprFolder::fold_relational_equality(
    const IOPCode iopc,
    const Expr *const left,
    const Expr *const right,
    const SourceLocation result_loc)

{
    DEBUG_SMART_ASSERT(
        !!left, !!right,
        SemUtils::is_static_expr(left),
        SemUtils::is_static_expr(right)
    );
    const bool left_value = SemUtils::as_bool(left);
    const bool right_value = SemUtils::as_bool(right);
    if (iopc == IOPCode::IF_EQ)
        return expr_maker_->make_const_bool_expr(result_loc, left_value == right_value);
    if (iopc == IOPCode::IF_NOTEQ)
        return expr_maker_->make_const_bool_expr(result_loc, left_value != right_value);
    throw std::logic_error(ATTACH_CONTEXT("Needed equality IOPCode"));
}

inline const Expr *
ExprFolder::fold_relational_arithmetic(
    const IOPCode iopc,
    const Expr *const left,
    const Expr *const right,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(
        !!left, !!right,
        SemUtils::is_const_arithmetic_expr(left),
        SemUtils::is_const_arithmetic_expr(right)
    );
    const auto fold_rel_op = [this, iopc, result_loc](const auto l, const auto r) -> const Expr *
    {
        switch (iopc)
        {
        case IOPCode::IF_GREATER:
            return expr_maker_->make_const_bool_expr(result_loc, l > r);
        case IOPCode::IF_GREATEREQ:
            return expr_maker_->make_const_bool_expr(result_loc, l >= r);
        case IOPCode::IF_LESS:
            return expr_maker_->make_const_bool_expr(result_loc, l < r);
        case IOPCode::IF_LESSEQ:
            return expr_maker_->make_const_bool_expr(result_loc, l <= r);
        default:
            throw std::logic_error(ATTACH_CONTEXT("Needed relational arithmetic IOPC"));
        }
    };

    return left->type == Expr::Type::CONST_INT && right->type == Expr::Type::CONST_INT
           ? fold_rel_op(static_cast<const ConstIntExpr *>(left)->value,
                         static_cast<const ConstIntExpr *>(right)->value)
           : fold_rel_op(extract_alpha_float(left), extract_alpha_float(right));
}

inline const Expr *ExprFolder::fold_logical_or(
    const Expr *const left,
    const Expr *const right,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!left, !!right);
    if (SemUtils::is_const_true_expr(left) || SemUtils::is_const_true_expr(right))
        return expr_maker_->make_const_bool_expr(result_loc, true);
    if (SemUtils::is_const_false_expr(left) && SemUtils::is_const_false_expr(right))
        return expr_maker_->make_const_bool_expr(result_loc, false);
    if (SemUtils::is_const_false_expr(left)) // false OR var = var
        return expr_maker_->clone_with_updated_location(result_loc, right);
    if (SemUtils::is_const_false_expr(right)) // var OR false = var
        return expr_maker_->clone_with_updated_location(result_loc, left);
    throw std::logic_error(ATTACH_CONTEXT(
        "This function should not be used, if at least one operand is not ConstBoolExpr"));
}

inline const Expr *ExprFolder::fold_logical_and(
    const Expr *const left,
    const Expr *const right,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!left, !!right);
    if (SemUtils::is_const_false_expr(left) || SemUtils::is_const_false_expr(right))
        return expr_maker_->make_const_bool_expr(result_loc, false);
    if (SemUtils::is_const_true_expr(left) && SemUtils::is_const_true_expr(right))
        return expr_maker_->make_const_bool_expr(result_loc, true);
    if (SemUtils::is_const_true_expr(left)) // true AND var = var
        return expr_maker_->clone_with_updated_location(result_loc, right);
    if (SemUtils::is_const_true_expr(right)) // var AND true = var
        return expr_maker_->clone_with_updated_location(result_loc, left);
    throw std::logic_error(ATTACH_CONTEXT(
        "This function should not be used, if at least one operand is not ConstBoolExpr"));
}

inline const Expr *ExprFolder::fold_logical_not(
    const Expr *const expr,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (!SemUtils::is_const_bool_expr(expr))
        throw std::logic_error(ATTACH_CONTEXT(
            "This function should not be used, if operand is not ConstBoolExpr"));

    return expr_maker_->make_const_bool_expr(
        result_loc, !static_cast<const ConstBoolExpr *>(expr)->value);
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
