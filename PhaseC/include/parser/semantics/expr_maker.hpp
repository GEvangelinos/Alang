//
// Created by stygian on 5/25/25.
//

#ifndef ALPHA_EXPR_MAKER_HPP
#define ALPHA_EXPR_MAKER_HPP

#include "expr_validation.hpp"
#include "core/alpha_basics.hpp"
#include "core/alpha_ir.hpp"
#include "core/alpha_location.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_semantic_driver.hpp"

namespace Alpha
{
class ExprMaker : private Immobile
{
public:
        explicit ExprMaker(ParseCtx &parse_ctx);

        ~ExprMaker() noexcept;

        [[nodiscard]] Expr *emit_quad_if_table_item(Expr *expr);

        [[nodiscard]] Expr *make_expr_variable(const Symbol *symbol, SourceLocation var_loc);

        [[nodiscard]] Expr *make_expr_const_string(const char *str_value, SourceLocation str_loc);

        [[nodiscard]] Expr *make_expr_const_real(f64 real_value, SourceLocation real_loc);

        [[nodiscard]] Expr *make_expr_const_int(i64 int_value, SourceLocation int_loc);

        [[nodiscard]] Expr *make_expr_const_bool(bool bool_value, SourceLocation bool_loc);

        [[nodiscard]] Expr *make_expr_const_nil(SourceLocation nil_loc);

        [[nodiscard]] Expr *make_expr_program_function(const Function *function_symbol);

        [[nodiscard]] Expr *make_expr_assign(
                const Expr *rvalue, SourceLocation assign_loc); // TODO: !! Why two make assign expr?
        [[nodiscard]] Expr *make_expr_assign(const Symbol *symbol,
                                             SourceLocation assign_loc); // TODO: WHy 2? make_assign_expr?
        [[nodiscard]] Expr *make_expr_new_table(SourceLocation new_table_loc);

        [[nodiscard]] Expr *make_expr_arithmetic(SourceLocation arithmetic_loc);

        [[nodiscard]] Expr *make_expr_boolean(SourceLocation bool_expr_loc);

        [[nodiscard]] Expr *make_expr_table_item(Expr *&lvalue, const std::string &id,
                                                 SourceLocation id_loc, SourceLocation table_item_loc);

        [[nodiscard]] Expr *make_expr_table_item(Expr *&lvalue, Expr *expr, SourceLocation table_tem_Loc);

private:
        std::vector<const Expr *> expr_sink_;
        ParseCtx &parse_ctx_;
};

inline ArithmeticBuilder::ArithmeticBuilder(SemanticDriver *const sd)
        : sd_(Utils::require_ptr(sd)),
          parse_ctx_(Utils::require_ptr(sd)->parse_ctx_) {}

inline Expr *ArithmeticBuilder::make_arithmetic(
        IOPCode iopc,
        Expr *left,
        Expr *right,
        SourceLocation result_loc,
        SourceLocation left_loc,
        SourceLocation right_loc)
{
        DEBUG_SMART_ASSERT(!!left, !!right);
        sd_->expr_validator_.report_if_not_arithmetic(iopc, left, left_loc, OperandSide::LEFT);
        sd_->expr_validator_.report_if_not_arithmetic(iopc, right, right_loc, OperandSide::RIGHT);

        if (sd_->sem_opts_.fold_arithmetic)
                if (SemUtils::is_const_numeric_expr(left) && SemUtils::is_const_numeric_expr(right))
                        if (Expr *folded = fold_arithmetic(iopc, left, right, result_loc))
                                return folded;

        Expr *result = parse_ctx_.expr_handler.make_expr_arithmetic(result_loc);
        parse_ctx_.quad_handler.emit_quad(iopc, left, right, result, result_loc);
        return result;
}
} // namespace Alpha
#endif //ALPHA_EXPR_MAKER_HPP
