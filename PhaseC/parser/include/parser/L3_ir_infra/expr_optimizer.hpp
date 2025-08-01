#ifndef EXPR_FOLDER_HPP
#define EXPR_FOLDER_HPP

#include <cmath>
#include "expr_maker.hpp"
#include "semantic_utils.hpp"
#include "core/source_location.hpp"
#include "parser/ir.hpp"

namespace Alpha
{
class ExprFolder
{
public:
    explicit ExprFolder(ExprMaker *expr_maker);

    [[nodiscard]] const Expr *fold_uminus(const Expr *expr, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_arithmetic(
        IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_relational_equality(
        IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_relational_arithmetic(
        IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_logical_or(
        const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_logical_and(
        const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *fold_logical_not(const Expr *expr, SourceLocation result_loc);

    [[nodiscard]] const Expr *try_simplify_relational_equality(
        IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

private:
    ExprMaker *const expr_maker_;
};

class ExprTrimmer
{
public:
    explicit ExprTrimmer(ExprMaker *expr_maker);

    [[nodiscard]] const Expr *try_trim_relational_equality(
        IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_trim_binary_arithmetic(
        IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

private:
    ExprMaker *const expr_maker_;

    [[nodiscard]] const Expr *try_trim_add(
        const Expr *lhs, const Expr *rhs, SourceLocation add_loc);
    [[nodiscard]] const Expr *try_trim_sub(
        const Expr *lhs, const Expr *rhs, SourceLocation sub_loc);
    [[nodiscard]] const Expr *try_trim_mul(
        const Expr *lhs, const Expr *rhs, SourceLocation mul_loc);
    [[nodiscard]] const Expr *try_trim_div(
        const Expr *lhs, const Expr *rhs, SourceLocation div_loc);
    [[nodiscard]] const Expr *try_trim_mod(
        const Expr *lhs, const Expr *rhs, SourceLocation mod_loc);
};

class ExprOptimizer
{
public:
    explicit ExprOptimizer(ExprMaker *expr_maker);

    ExprFolder expr_folder;
    ExprTrimmer expr_trimmer;
};

inline ExprOptimizer::ExprOptimizer(ExprMaker *expr_maker)
    : expr_folder(Utils::require_ptr(expr_maker)),
      expr_trimmer(Utils::require_ptr(expr_maker)) {}

inline ExprFolder::ExprFolder(ExprMaker *const expr_maker)
    : expr_maker_(Utils::require_ptr(expr_maker)) {}

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
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(
        !!lhs, !!rhs,
        SemUtils::is_const_arithmetic_expr(lhs),
        SemUtils::is_const_arithmetic_expr(rhs)
    );

    const auto fold_arith_op = [this, iopc, result_loc](const auto l, const auto r) -> const Expr *
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

    return lhs->type == Expr::Type::CONST_INT && rhs->type == Expr::Type::CONST_INT
           ? fold_arith_op(static_cast<const ConstIntExpr *>(lhs)->value,
                           static_cast<const ConstIntExpr *>(rhs)->value)
           : fold_arith_op(SemUtils::extract_alpha_float(lhs), SemUtils::extract_alpha_float(rhs));
}

inline const Expr *
ExprFolder::fold_relational_equality(
    const IOPCode iopc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)

{
    DEBUG_SMART_ASSERT(
        !!lhs, !!rhs,
        iopc == IOPCode::IF_EQ || iopc == IOPCode::IF_NEQ,
        (SemUtils::is_static_expr(lhs) || SemUtils::is_static_expr(rhs))
        && "Expected at least 1 static_expr, for at least partial folding"
    );

    if (SemUtils::is_static_expr(lhs) && SemUtils::is_static_expr(rhs))
    {
        const bool lhs_value = SemUtils::as_bool(lhs);
        const bool rhs_value = SemUtils::as_bool(rhs);
        if (iopc == IOPCode::IF_EQ)
            return expr_maker_->make_const_bool_expr(result_loc, lhs_value == rhs_value);
        if (iopc == IOPCode::IF_NEQ)
            return expr_maker_->make_const_bool_expr(result_loc, lhs_value != rhs_value);
        throw std::logic_error(ATTACH_CONTEXT("Needed equality IOPCode"));
    }
}

inline const Expr *
ExprFolder::fold_relational_arithmetic(
    const IOPCode iopc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(
        !!lhs, !!rhs,
        SemUtils::is_const_arithmetic_expr(lhs),
        SemUtils::is_const_arithmetic_expr(rhs)
    );
    const auto fold_rel_op = [this, iopc, result_loc](const auto l, const auto r) -> const Expr *
    {
        switch (iopc)
        {
        case IOPCode::IF_GT:
            return expr_maker_->make_const_bool_expr(result_loc, l > r);
        case IOPCode::IF_GTE:
            return expr_maker_->make_const_bool_expr(result_loc, l >= r);
        case IOPCode::IF_LT:
            return expr_maker_->make_const_bool_expr(result_loc, l < r);
        case IOPCode::IF_LTE:
            return expr_maker_->make_const_bool_expr(result_loc, l <= r);
        default:
            throw std::logic_error(ATTACH_CONTEXT("Needed relational arithmetic IOPC"));
        }
    };

    return lhs->type == Expr::Type::CONST_INT && rhs->type == Expr::Type::CONST_INT
           ? fold_rel_op(static_cast<const ConstIntExpr *>(lhs)->value,
                         static_cast<const ConstIntExpr *>(rhs)->value)
           : fold_rel_op(SemUtils::extract_alpha_float(lhs), SemUtils::extract_alpha_float(rhs));
}

inline const Expr *ExprFolder::fold_logical_or(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (SemUtils::is_const_true_expr(lhs) || SemUtils::is_const_true_expr(rhs))
        return expr_maker_->make_const_bool_expr(result_loc, true);
    if (SemUtils::is_const_false_expr(lhs) && SemUtils::is_const_false_expr(rhs))
        return expr_maker_->make_const_bool_expr(result_loc, false);
    if (SemUtils::is_const_false_expr(lhs)) // false OR var = var
        return expr_maker_->clone_with_updated_location(result_loc, rhs);
    if (SemUtils::is_const_false_expr(rhs)) // var OR false = var
        return expr_maker_->clone_with_updated_location(result_loc, lhs);
    throw std::logic_error(ATTACH_CONTEXT(
        "This function should not be used, if at least one operand is not ConstBoolExpr"));
}

inline const Expr *ExprFolder::fold_logical_and(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (SemUtils::is_const_false_expr(lhs) || SemUtils::is_const_false_expr(rhs))
        return expr_maker_->make_const_bool_expr(result_loc, false);
    if (SemUtils::is_const_true_expr(lhs) && SemUtils::is_const_true_expr(rhs))
        return expr_maker_->make_const_bool_expr(result_loc, true);
    if (SemUtils::is_const_true_expr(lhs)) // true AND var = var
        return expr_maker_->clone_with_updated_location(result_loc, rhs);
    if (SemUtils::is_const_true_expr(rhs)) // var AND true = var
        return expr_maker_->clone_with_updated_location(result_loc, lhs);
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

inline ExprTrimmer::ExprTrimmer(ExprMaker *expr_maker)
    : expr_maker_(expr_maker) {}

inline const Expr *
ExprTrimmer::try_trim_relational_equality(
    const IOPCode iopc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    if (iopc == IOPCode::IF_NEQ) // var != 0 -> var     <<<>>>     0 != var -> var
    {
        if (SemUtils::is_static_expr(lhs) && SemUtils::as_bool(lhs) == false)
            return expr_maker_->clone_with_updated_location(result_loc, rhs);
        if (SemUtils::is_static_expr(rhs) && SemUtils::as_bool(rhs) == false)
            return expr_maker_->clone_with_updated_location(result_loc, lhs);
    }
    if (iopc == IOPCode::IF_EQ)
    {
        // 1 == var(true) -> var(true), 1 == var(false) -> var(false) => 1 == var -> var
        if (SemUtils::is_const_expr_true_or_1(lhs) && SemUtils::is_const_bool_expr(rhs))
            return expr_maker_->clone_with_updated_location(result_loc, rhs);
        if (SemUtils::is_const_expr_true_or_1(rhs) && SemUtils::is_const_bool_expr(lhs))
            return expr_maker_->clone_with_updated_location(result_loc, lhs);
    }
    return nullptr; // Simplification failed.
}

inline const Expr *
ExprTrimmer::try_trim_binary_arithmetic(
    const IOPCode iopc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    switch (iopc)
    {
    case IOPCode::ADD: return try_trim_add(lhs, rhs, result_loc);
    case IOPCode::SUB: return try_trim_sub(lhs, rhs, result_loc);
    case IOPCode::MUL: return try_trim_mul(lhs, rhs, result_loc);
    case IOPCode::DIV: return try_trim_div(lhs, rhs, result_loc);
    case IOPCode::MOD: return try_trim_mod(lhs, rhs, result_loc);
        [[unlikely]] default: throw std::logic_error(
            ATTACH_CONTEXT("Expected a binary arithmetic IOPCode"));
    }
}

inline const Expr *
ExprTrimmer::try_trim_add(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation add_loc)
{
    DEBUG_SMART_ASSERT(
        !(SemUtils::is_const_arithmetic_expr(lhs) && SemUtils::is_const_arithmetic_expr(rhs))
        && "try_trim_add: both operands are const; should be folded by ExprFolder."
    );

    // 0 + x -> x and x + 0 -> x
    if (SemUtils::is_const_0(lhs)) return expr_maker_->clone_with_updated_location(add_loc, rhs);
    if (SemUtils::is_const_0(rhs)) return expr_maker_->clone_with_updated_location(add_loc, lhs);

    // Trimming failed (most common scenario)
    return nullptr;
}

inline const Expr *
ExprTrimmer::try_trim_sub(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation sub_loc)
{
    DEBUG_SMART_ASSERT(
        !(SemUtils::is_const_arithmetic_expr(lhs) && SemUtils::is_const_arithmetic_expr(rhs))
        && "try_trim_add: both operands are const; should be folded by ExprFolder."
    );

    // x - 0 -> x
    if (SemUtils::is_const_0(rhs)) return expr_maker_->clone_with_updated_location(sub_loc, lhs);

    // Trimming failed (most common scenario)
    return nullptr;
}

inline const Expr *
ExprTrimmer::try_trim_mul(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation mul_loc)
{
    DEBUG_SMART_ASSERT(
        !(SemUtils::is_const_arithmetic_expr(lhs) && SemUtils::is_const_arithmetic_expr(rhs))
        && "try_trim_add: both operands are const; should be folded by ExprFolder."
    );

    // x * 0 -> 0 and 0 * x -> 0
    if (SemUtils::is_const_0(lhs))return expr_maker_->make_const_int_expr(mul_loc, 0);
    if (SemUtils::is_const_0(rhs)) return expr_maker_->make_const_int_expr(mul_loc, 0);

    // x * 1 -> x and 1 * x -> x
    if (SemUtils::is_const_1(lhs)) return expr_maker_->clone_with_updated_location(mul_loc, rhs);
    if (SemUtils::is_const_1(rhs)) return expr_maker_->clone_with_updated_location(mul_loc, lhs);

    // Trimming failed (most common scenario)
    return nullptr;
}

inline const Expr *
ExprTrimmer::try_trim_div(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation div_loc)
{
    DEBUG_SMART_ASSERT(
        !(SemUtils::is_const_arithmetic_expr(lhs) && SemUtils::is_const_arithmetic_expr(rhs))
        && "try_trim_add: both operands are const; should be folded by ExprFolder."
    );

    if (SemUtils::is_const_1(rhs)) return expr_maker_->clone_with_updated_location(div_loc, lhs);

    // Trimming failed (most common scenario)
    return nullptr;
}

inline const Expr *
ExprTrimmer::try_trim_mod(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation mod_loc)
{
    DEBUG_SMART_ASSERT(
        !(SemUtils::is_const_arithmetic_expr(lhs) && SemUtils::is_const_arithmetic_expr(rhs))
        && "try_trim_add: both operands are const; should be folded by ExprFolder."
    );

    // x % 1 -> 0
    if (SemUtils::is_const_1(rhs)) return expr_maker_->make_const_int_expr(mod_loc, 0);

    // Trimming failed (most common scenario)
    return nullptr;
}
} // namespace Alpha

#endif //EXPR_FOLDER_HPP
