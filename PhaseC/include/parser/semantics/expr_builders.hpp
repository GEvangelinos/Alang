//
// Created by stygian on 5/25/25.
//

#ifndef ALPHA_EXPR_MAKER_HPP
#define ALPHA_EXPR_MAKER_HPP

#include "core/alpha_location.hpp"
#include  "core/alpha_ir.hpp"
#include "parser/alpha_parser_context.hpp"

namespace Alpha
{
class ArithmeticBuilder : private Immobile
{
public:
        explicit ArithmeticBuilder(const ParseCtx &parse_ctx);

        [[nodiscard]] Expr *make_arithmetic(
                IOPCode iopc,
                Expr *left,
                Expr *right,
                SourceLocation result_loc,
                SourceLocation left_loc,
                SourceLocation right_loc);

private:
        const ParseCtx &parse_ctx_;
};
} // namespace Alpha
#endif //ALPHA_EXPR_MAKER_HPP
