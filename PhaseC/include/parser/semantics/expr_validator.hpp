#ifndef EXPR_VALIDATOR_HPP
#define EXPR_VALIDATOR_HPP

#include "semantic_utils.hpp"
#include "core/alpha_error.hpp"
#include "core/alpha_ir.hpp"

namespace Alpha
{
class ExprValidator
{
public:
        ExprValidator(ErrorTracker *error_tracker);

        void report_if_not_arithmetic(
                IOPCode iopc,
                const Expr *expr,
                SourceLocation expr_loc,
                OperandSide op_pos);

private:
        ErrorTracker *const error_tracker_;

        void report_non_arithmetic_operand(
                IOPCode iopc,
                const Expr *expr,
                SourceLocation expr_loc,
                OperandSide operand_pos) const;
};

inline void
ExprValidator::report_if_not_arithmetic(
        const IOPCode iopc,
        const Expr *expr,
        const SourceLocation expr_loc,
        const OperandSide op_pos)
{
        DEBUG_SMART_ASSERT(!!expr);
        if(!SemUtils::is_numeric_convertible_expr(expr))
                report_non_arithmetic_operand(iopc, expr, expr_loc, op_pos);
}

inline void
ExprValidator::report_non_arithmetic_operand(
        const IOPCode iopc,
        const Expr *expr,
        const SourceLocation expr_loc,
        const OperandSide operand_pos) const
{
        std::string error;
        using OP = OperandSide;
        if(operand_pos == OP::LEFT || operand_pos == OP::RIGHT)
                error = FMT::format("`{}` operand of arithmetic operator `{}` is never arithmetic ",
                                    to_string(operand_pos),
                                    SemUtils::relational_iopcode_to_string_symbol(iopc));
        else if(operand_pos == OP::UNARY)
                error = "operand of unary `-` is never arithmetic";
        else [[unlikely]]
                throw std::logic_error(ATTACH_CONTEXT("Invalid arithmetic OperandSide"));

        const std::string note =
                FMT::format("operand's expression type: `{}`", to_string(expr->type));

        error_tracker_->report_error(CTError::Type::SEMANTIC, error, expr_loc, note, expr_loc);
}
} // namespace Alpha
#endif // EXPR_VALIDATOR_HPP
