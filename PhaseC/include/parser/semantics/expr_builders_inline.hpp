//
// Created by stygian on 5/25/25.
//
#ifndef EXPR_BUILDER_INLINE_HPP
#define EXPR_BUILDER_INLINE_HPP
#include "expr_builders.hpp"

namespace Alpha
{
inline ArithmeticBuilder::ArithmeticBuilder(const ParseCtx &parse_ctx)
        : parse_ctx_(parse_ctx) {}

inline Expr *ArithmeticBuilder::make_arithmetic(IOPCode iopc, Expr *left, Expr *right,
                                                SourceLocation result_loc, SourceLocation left_loc,
                                                SourceLocation right_loc)
{
        DEBUG_SMART_ASSERT(!!left, !!right);

        // report_error_if_not_arithmetic(iopc, left, left_loc, OperandPosition::LEFT);
        // report_error_if_not_arithmetic(iopc, right, right_loc, OperandPosition::RIGHT);

        // TODO: 1. Rename_try_folding to fold. 2. make a checker function to see if folding is
        // doable 3. cleanup.. you wrote this comment 36 minutes before DEADLINE.. CAUSE you saw
        // x = a+b returning 0. It was because you initialized all unused field to 0.

        // TODO: make a more advance arithmetic folding.. for example 1 +x +2 + 3 +4
        // wont constant fold currently as evaluation is left_to_right.
        // and 1+x becomes airthmetic... and we there continue with arithmetic_Expr +constant_num.
        if((left->type == Expr::Type::CONST_INT || left->type == Expr::Type::CONST_REAL) &&
           (right->type == Expr::Type::CONST_INT || right->type == Expr::Type::CONST_REAL))
                if(sem_opts.arithmetic_folding)
                        if(Expr *folded = try_fold_arithmetic(iopc, left, right, result_loc))
                                return folded;

        Expr *result = parse_ctx_.expr_handler.make_expr_arithmetic(result_loc);
        parse_ctx_.quad_handler.emit_quad(iopc, left, right, result, result_loc);
        return result;
}
}

#endif //EXPR_BUILDER_INLINE_HPP
