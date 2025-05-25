//
// Created by stygian on 5/25/25.
//

#ifndef ALPHA_EXPR_MAKER_HPP
#define ALPHA_EXPR_MAKER_HPP

#include "expr_validator.hpp"
#include "core/alpha_basics.hpp"
#include "core/alpha_ir.hpp"
#include "core/alpha_location.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_semantic_driver.hpp"

namespace Alpha
{
class ArithmeticBuilder : private Immobile
{
public:
        explicit ArithmeticBuilder(SemanticDriver *sm);

        [[nodiscard]] Expr *make_arithmetic(
                IOPCode iopc,
                Expr *left,
                Expr *right,
                SourceLocation result_loc,
                SourceLocation left_loc,
                SourceLocation right_loc);

private:
        SemanticDriver *const sd_;
        ParseCtx *const parse_ctx_;
};

inline ArithmeticBuilder::ArithmeticBuilder(SemanticDriver *const sd)
        : sd_(sd),
        parse_ctx_(sd->parse_ctx_)
{
        // Not in hot path. We can afford non DEBUG assertion.
        SMART_ASSERT(!!sd);
        SMART_ASSERT(!!sd->parse_ctx_);
}

inline Expr *ArithmeticBuilder::make_arithmetic(
        IOPCode iopc,
        Expr *left,
        Expr *right,
        SourceLocation result_loc,
        SourceLocation left_loc,
        SourceLocation right_loc)
{
        DEBUG_SMART_ASSERT(!!left, !!right);

        report_error_if_not_arithmetic(iopc, left, left_loc, OperandPosition::LEFT);
        report_error_if_not_arithmetic(iopc, right, right_loc, OperandPosition::RIGHT);

        if(sem_opts_.fold_arithmetic)
                if(SemUtils::is_const_number_expr(left) && SemUtils::is_const_number_expr(right))

                        if(Expr *folded = try_fold_arithmetic(iopc, left, right, result_loc))
                                return folded;

        Expr *result = parse_ctx_.expr_handler.make_expr_arithmetic(result_loc);
        parse_ctx_.quad_handler.emit_quad(iopc, left, right, result, result_loc);
        return result;
}
} // namespace Alpha
#endif //ALPHA_EXPR_MAKER_HPP
