#ifndef EXPR_SNITCH_HPP
#define EXPR_SNITCH_HPP

#include "parser/semantic_utils.hpp"
#include "parser/ir.hpp"
#include "diagnostics/diagnostic_engine.hpp"

namespace Alpha
{
class ExprSnitch
{
public:
    explicit ExprSnitch(DiagnosticReporter *dr);

    void report_if_not_arithmetic_expr(IOPCode iopc, const Expr *expr, OperandSide op_side);
    void report_if_not_relational(IOPCode iopc, const Expr *expr, OperandSide op_side);
    void report_if_int_to_float_loss(AlphaInt int_value, SourceLocation conversion_loc);

private:
    DiagnosticReporter *const dr_;

    void report_non_arithmetic_operand(IOPCode iopc, const Expr *expr, OperandSide op_side);
};

inline
ExprSnitch::ExprSnitch(DiagnosticReporter *const dr)
    : dr_(Utils::require_ptr(dr)) {}

inline void
ExprSnitch::report_if_not_arithmetic_expr(
    const IOPCode iopc,
    const Expr *expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (!SemUtils::is_arithmetic_convertible_expr(expr))
        report_non_arithmetic_operand(iopc, expr, op_side);
}

inline void
ExprSnitch::report_non_arithmetic_operand(
    const IOPCode iopc,
    const Expr *const expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (SemUtils::is_binary_arithmetic_iopcode(iopc))
        dr_->report_arith_op_nonarith_operand(
            op_side, SemUtils::arith_op_to_str(iopc), expr->type, expr->loc);
    else if (iopc == IOPCode::UMINUS)
        dr_->report_uminus_nonarith_operand(expr->type, expr->loc);
    else
        throw std::logic_error(ATTACH_CONTEXT("Expected arithmetic IOPCode (bin arith or uminus)"));
}

inline void
ExprSnitch::report_if_not_relational(
    const IOPCode iopc,
    const Expr *const expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr,);

    // In Alpha everything is convertible to bool.
    // And operators == and != convert their operands to bool.
    if (SemUtils::is_relational_equality_iopcode(iopc))
        return;
    // If here relational operator is:  < <= > >=
    if (SemUtils::is_arithmetic_convertible_expr(expr))
        return;
    dr_->report_rel_op_nonarith_operand(
        op_side, SemUtils::rel_op_to_str(iopc), expr->type, expr->loc);
}

inline void
ExprSnitch::report_if_int_to_float_loss(
    const AlphaInt int_value,
    const SourceLocation conversion_loc)
{
    if (!Utils::is_lossless_int_to_float<AlphaFloat>(int_value))
        dr_->report_implicit_int_to_float_loss(conversion_loc);
}
} // namespace Alpha
#endif // EXPR_SNITCH_HPP
