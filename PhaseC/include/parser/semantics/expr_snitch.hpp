#ifndef EXPR_SNITCH_HPP
#define EXPR_SNITCH_HPP

#include "semantic_utils.hpp"
#include "core/alpha_core_types.hpp"
#include "core/alpha_diagnostics.hpp"

namespace Alpha
{
class ExprSnitch
{
public:
    ExprSnitch(Diagnostics *diagnostics);

    void report_if_not_arithmetic(IOPCode iopc, const Expr *expr, SourceLocation expr_loc,
                                  OperandSide op_side);
    void report_if_not_relational(IOPCode iopc, const Expr *expr, SourceLocation expr_loc,
                                  OperandSide op_side);
    void report_if_int_to_float_loss(AlphaInt int_value, SourceLocation conversion_loc);

private:
    Diagnostics *const diagnostics_;

    void report_non_arithmetic_operand(IOPCode iopc, const Expr *expr, SourceLocation expr_loc,
                                       OperandSide op_side);
    void report_non_relational_operand(IOPCode iopc, const Expr *expr, SourceLocation expr_loc,
                                       OperandSide op_side);
};

inline
ExprSnitch::ExprSnitch(Diagnostics *const diagnostics)
    : diagnostics_(Utils::require_ptr(diagnostics)) {}

inline void
ExprSnitch::report_if_not_arithmetic(
    const IOPCode iopc,
    const Expr *expr,
    const SourceLocation expr_loc,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (!SemUtils::is_arithmetic_convertible_expr(expr))
        report_non_arithmetic_operand(iopc, expr, expr_loc, op_side);
}

inline void
ExprSnitch::report_non_arithmetic_operand(
    const IOPCode iopc,
    const Expr *expr,
    const SourceLocation expr_loc,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr);
    std::string error;
    if (op_side == OperandSide::LEFT || op_side == OperandSide::RIGHT)
        error = FMT::format("`{}` operand of arithmetic operator `{}` is never arithmetic ",
                            to_string(op_side), SemUtils::relop_to_str(iopc));
    else if (op_side == OperandSide::UNARY)
        error = "operand of unary `-` is never arithmetic";
    else [[unlikely]]
        throw std::logic_error(ATTACH_CONTEXT("Invalid arithmetic OperandSide"));
    const std::string note = FMT::format("operand's expression type: `{}`", to_string(expr->type)); // NOLINT
    diagnostics_->report(Issue::Type::ERROR, error, expr_loc, std::list{Note{note, expr_loc}});
}

inline void ExprSnitch::report_if_not_relational(
    const IOPCode iopc,
    const Expr *const expr,
    const SourceLocation expr_loc,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(
        !!expr,
        SemUtils::is_relational_iopcode(iopc),
        op_side == OperandSide::LEFT || op_side == OperandSide::RIGHT
    );

    // In Alpha everything is convertible to bool.
    // And operators == and != convert their operands to bool.
    if (SemUtils::is_relational_equality_iopcode(iopc))
        return;
    // If here operator IOPCode is:  < <= > >=
    if (SemUtils::is_arithmetic_convertible_expr(expr))
        return;
    report_non_relational_operand(iopc, expr, expr_loc, op_side);
}

inline void ExprSnitch::report_non_relational_operand(
    const IOPCode iopc,
    const Expr *expr,
    const SourceLocation expr_loc,
    const OperandSide op_side)
{
    if (op_side != OperandSide::LEFT && op_side != OperandSide::RIGHT) [[unlikely]]
        throw std::logic_error(ATTACH_CONTEXT("Invalid relational OperandSide "));

    const std::string error =
        FMT::format("`{}` operand of relational operator `{}` is never arithmetic",
                    to_string(op_side), SemUtils::relop_to_str(iopc));
    const std::string note = FMT::format("operand's expression type: `{}`", to_string(expr->type));
    diagnostics_->report(Issue::Type::ERROR, error, expr_loc, std::list{Note{note, expr_loc}});
}

inline void
ExprSnitch::report_if_int_to_float_loss(AlphaInt int_value, SourceLocation conversion_loc)
{
    if (Utils::is_lossless_int_to_float<AlphaFloat>(int_value))
        return;
    diagnostics_->report(
        Issue::Type::WARNING,
        "integer to floating-point implicit conversion results in integral precision loss",
        conversion_loc);
}
} // namespace Alpha
#endif // EXPR_SNITCH_HPP
