#ifndef EXPR_BUILDERS_HPP
#define EXPR_BUILDERS_HPP

#include "expr_folder.hpp"
#include "expr_maker.hpp"
#include "quad_handler.hpp"
#include "semantic_utils.hpp"
#include "core/alpha_core_types.hpp"

namespace Alpha
{
class BasicBuilder
{
public:
    struct Options
    {
        bool fold_arithmetic;
        bool fold_relational;
        bool fold_logical;
    };

    BasicBuilder(
        Options &&options,
        ExprSnitch *snitch,
        ExprMaker *expr_maker,
        ExprFolder *expr_folder,
        QuadHandler *quad_handler);

    [[nodiscard]] const Expr *build_arithmetic(
        IOPCode iopc, const Expr *left, const Expr *right,
        SourceLocation left_loc, SourceLocation right_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_uminus(
        const Expr *expr, SourceLocation term_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_relational(
        IOPCode iopc, const Expr *left, const Expr *right,
        SourceLocation left_loc, SourceLocation right_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_or(
        const Expr *left, const Expr *right,
        SourceLocation left_loc, SourceLocation right_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_and(
        const Expr *left, const Expr *right,
        SourceLocation left_loc, SourceLocation right_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_not(const Expr *expr, SourceLocation result_loc);

private:
    const Options options_;
    ExprSnitch *const snitch_;
    ExprMaker *const expr_maker_;
    ExprFolder *const expr_folder_;
    QuadHandler *const quad_handler_;
};

inline BasicBuilder::BasicBuilder(
    Options &&options,
    ExprSnitch *const snitch,
    ExprMaker *const expr_maker,
    ExprFolder *const expr_folder,
    QuadHandler *const quad_handler)
    : options_(std::move(options)),
      snitch_(Utils::require_ptr(snitch)),
      expr_maker_(Utils::require_ptr(expr_maker)),
      expr_folder_(Utils::require_ptr(expr_folder)),
      quad_handler_(Utils::require_ptr(quad_handler)) {}

inline const Expr *
BasicBuilder::build_arithmetic(
    const IOPCode iopc,
    const Expr *const left,
    const Expr *const right,
    const SourceLocation left_loc,
    const SourceLocation right_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!left, !!right);
    snitch_->report_if_not_arithmetic(iopc, left, left_loc, OperandSide::LEFT);
    snitch_->report_if_not_arithmetic(iopc, right, right_loc, OperandSide::RIGHT);

    if (options_.fold_arithmetic &&
        SemUtils::is_const_arithmetic_expr(left) &&
        SemUtils::is_const_arithmetic_expr(right))
        return expr_folder_->fold_arithmetic(iopc, left, right, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next_quad(iopc, left, right, arithmetic_expr, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::build_uminus(
    const Expr *const expr,
    const SourceLocation term_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    snitch_->report_if_not_arithmetic(IOPCode::UMINUS, expr, term_loc, OperandSide::UNARY);

    if (options_.fold_arithmetic && SemUtils::is_const_arithmetic_expr(expr))
        return expr_folder_->fold_uminus(expr, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(term_loc);
    quad_handler_->emit_next_quad(IOPCode::UMINUS, expr, nullptr, arithmetic_expr, term_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::build_relational(
    const IOPCode iopc,
    const Expr *const left,
    const Expr *const right,
    const SourceLocation left_loc,
    const SourceLocation right_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!left, !!right);
    snitch_->report_if_not_relational(iopc, left, left_loc, OperandSide::LEFT);
    snitch_->report_if_not_relational(iopc, right, right_loc, OperandSide::RIGHT);

    if (options_.fold_relational &&
        SemUtils::is_relational_equality_iopcode(iopc) &&
        SemUtils::is_rvalue_expr(left) &&
        SemUtils::is_rvalue_expr(right))
        return expr_folder_->fold_relational_equality(iopc, left, right, result_loc);
    if (options_.fold_relational &&
        SemUtils::is_relational_arithmetic_iopcode(iopc) &&
        SemUtils::is_const_arithmetic_expr(left) &&
        SemUtils::is_const_arithmetic_expr(right))
        return expr_folder_->fold_relational_arithmetic(iopc, left, right, result_loc);

    BoolExpr *result_expr = expr_maker_->make_bool_expr(result_loc);

    result_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(iopc, left, right, nullptr, result_loc);
    result_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(IOPCode::JUMP, nullptr, nullptr, nullptr, result_loc);
    return result_expr;
}

// inline const Expr *
// BasicBuilder::build_logical_or(
//     const Expr *const left,
//     const Expr *const right,
//     const SourceLocation left_loc,
//     const SourceLocation right_loc,
//     const SourceLocation result_loc)
// {
//     // Check your solution on GitHub (latest commit on branch feature/ir-gen) (23/05/2025)
//     UNIMPLEMENTED();
// }
//
// inline const Expr *
// BasicBuilder::build_logical_and(
//     const Expr *left, const Expr *right,
//     SourceLocation left_loc, SourceLocation right_loc, SourceLocation result_loc)
// {
//     // Check your solution on GitHub (latest commit on branch feature/ir-gen) (23/05/2025)
//     UNIMPLEMENTED();
// }
//
// inline const Expr *
// BasicBuilder::build_logical_not(const Expr *expr, SourceLocation result_loc)
// {
//     // Check your solution on GitHub (latest commit on branch feature/ir-gen) (23/05/2025)
//     UNIMPLEMENTED();
// }
} // namespace Alpha
#endif // EXPR_BUILDERS_HPP
