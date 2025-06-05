// TODO: make const_bool_true, const_bool_false, const_int = 1 static expressions and reusable.
// Also you should you should be able to modify their loc, so QuadHandler can take their latest
// loc of use, and append it to the quad it is making.

#ifndef ALPHA_EXPR_MAKER_HPP
#define ALPHA_EXPR_MAKER_HPP

#include "expr_snitch.hpp"
#include "core/alpha_basics.hpp"
#include "core/alpha_location.hpp"
#include "parser/alpha_parser_context.hpp"

namespace Alpha
{
class ExprMaker : private Immobile
{
public:
    // Frequently used Const expressions.
    const ConstBoolExpr *const premade_true;
    const ConstBoolExpr *const premade_false;
    const ConstIntExpr *const premade_int_1;

    explicit ExprMaker(ParseCtx *parse_ctx);
    ~ExprMaker() noexcept;

    /// @Note: All expr_loc(s) are the loc of the resulting expression.
    [[nodiscard]] const ArithmeticExpr *make_arithmetic_expr(SourceLocation expr_loc);
    [[nodiscard]] const AssignExpr *make_assign_expr(const Expr *rvalue, SourceLocation expr_loc);
    [[nodiscard]] BoolExpr *make_bool_expr(SourceLocation expr_loc);
    [[nodiscard]] const ConstBoolExpr *make_const_bool_expr(
        bool bool_value, SourceLocation expr_loc);
    [[nodiscard]] const ConstIntExpr *make_const_int_expr(AlphaInt int_value,
                                                          SourceLocation expr_loc);
    [[nodiscard]] const ConstFloatExpr *make_const_float_expr(
        AlphaFloat float_value, SourceLocation expr_loc);
    [[nodiscard]] const ConstStringExpr *make_const_string_expr(
        const char *str_value, SourceLocation expr_loc);
    [[nodiscard]] const ConstNilExpr *make_nil_expr(SourceLocation expr_loc);
    [[nodiscard]] const NewTableExpr *make_new_table_expr(SourceLocation expr_loc);
    [[nodiscard]] const ProgFuncExpr *make_program_function_expr(
        SourceLocation expr_loc, const Function *function_symbol);
    [[nodiscard]] const TableItemExpr *make_table_item_expr(const Expr *lvalue, const Expr *index,
                                                            SourceLocation expr_loc);
    [[nodiscard]] const VariableExpr *make_variable_expr(const Symbol *symbol,
                                                         SourceLocation expr_loc);

private:
    ParseCtx *const parse_ctx_;
    std::vector<const Expr *> expr_sink_;
};

inline
ExprMaker::ExprMaker(ParseCtx *const parse_ctx)
    : premade_true(make_const_bool_expr(true, k_no_location)),
      premade_false(make_const_bool_expr(false, k_no_location)),
      premade_int_1(make_const_int_expr(1, k_no_location)),
      parse_ctx_(Utils::require_ptr(parse_ctx)) {}

inline ExprMaker::~ExprMaker() noexcept
{
    for (const Expr *e : expr_sink_)
    {
        DEBUG_SMART_ASSERT(!!e);
        switch (e->type)
        {
            // clang-format off
        case Expr::Type::ARITHMETIC_EXPR: delete static_cast<const ArithmeticExpr *>(e); break;
        case Expr::Type::ASSIGN_EXPR: delete static_cast<const AssignExpr *>(e);         break;
        case Expr::Type::BOOL_EXPR: delete static_cast<const BoolExpr *>(e);             break;
        case Expr::Type::CONST_BOOL: delete static_cast<const ConstBoolExpr *>(e);       break;
        case Expr::Type::CONST_INT: delete static_cast<const ConstIntExpr *>(e);         break;
        case Expr::Type::CONST_FLOAT: delete static_cast<const ConstFloatExpr *>(e);     break;
        case Expr::Type::CONST_STRING: delete static_cast<const ConstStringExpr *>(e);   break;
        case Expr::Type::CONST_NIL: delete static_cast<const ConstNilExpr *>(e);         break;
        case Expr::Type::LIBRARY_FUNCTION: delete static_cast<const LibFuncExpr *>(e);   break;
        case Expr::Type::PROGRAM_FUNCTION: delete static_cast<const ProgFuncExpr *>(e);  break;
        case Expr::Type::NEW_TABLE: delete static_cast<const NewTableExpr *>(e);         break;
        case Expr::Type::TABLE_ITEM: delete static_cast<const TableItemExpr *>(e);       break;
        case Expr::Type::VARIABLE: delete static_cast<const VariableExpr *>(e);          break;
        // clang-format on
        default: UNREACHABLE(FMT::format(
                "Unknown Expr::Type. Expr::Type's int value = {}", static_cast<int>(e->type)));
        }
    }
}

inline const ArithmeticExpr *
ExprMaker::make_arithmetic_expr(const SourceLocation expr_loc)
{
    const auto arithmetic_expr = new const ArithmeticExpr(
        expr_loc, parse_ctx_->new_temp());
    expr_sink_.push_back(arithmetic_expr);
    return arithmetic_expr;
}

inline const AssignExpr *
ExprMaker::make_assign_expr(const Expr *const rvalue, const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!rvalue);
    const auto assign_expr = new const AssignExpr(expr_loc, rvalue);
    expr_sink_.push_back(assign_expr);
    return assign_expr;
}

inline BoolExpr *ExprMaker::make_bool_expr(const SourceLocation expr_loc)
{
    const auto bool_expr = new BoolExpr(expr_loc, parse_ctx_->new_temp());
    expr_sink_.push_back(bool_expr);
    return bool_expr;
}

inline const ConstBoolExpr *
ExprMaker::make_const_bool_expr(const bool bool_value, const SourceLocation expr_loc)
{
    const auto const_bool_expr = new const ConstBoolExpr(expr_loc, bool_value);
    expr_sink_.push_back(const_bool_expr);
    return const_bool_expr;
}

inline const ConstIntExpr *
ExprMaker::make_const_int_expr(const AlphaInt int_value, const SourceLocation expr_loc)
{
    const auto const_int_expr = new const ConstIntExpr(expr_loc, int_value);
    expr_sink_.push_back(const_int_expr);
    return const_int_expr;
}

inline const ConstFloatExpr *
ExprMaker::make_const_float_expr(const AlphaFloat float_value, const SourceLocation expr_loc)
{
    const auto const_float_expr = new const ConstFloatExpr(expr_loc, float_value);
    expr_sink_.push_back(const_float_expr);
    return const_float_expr;
}

inline const ConstStringExpr *
ExprMaker::make_const_string_expr(const char *const str_value, const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!str_value);
    const auto const_str_expr = new const ConstStringExpr(expr_loc, str_value);
    expr_sink_.push_back(const_str_expr);
    return const_str_expr;
}

inline const ConstNilExpr *
ExprMaker::make_nil_expr(const SourceLocation expr_loc)
{
    const auto nil_expr = new const ConstNilExpr(expr_loc);
    expr_sink_.push_back(nil_expr);
    return nil_expr;
}

inline const NewTableExpr *ExprMaker::make_new_table_expr(const SourceLocation expr_loc)
{
    const auto new_table_expr = new const NewTableExpr(expr_loc, parse_ctx_->new_temp());
    expr_sink_.push_back(new_table_expr);
    return new_table_expr;
}

inline const ProgFuncExpr *
ExprMaker::make_program_function_expr(
    const SourceLocation expr_loc,
    const Function *const function_symbol)
{
    DEBUG_SMART_ASSERT(!!function_symbol);
    const auto progfunc_expr = new const ProgFuncExpr(expr_loc, function_symbol);
    expr_sink_.push_back(progfunc_expr);
    return progfunc_expr;
}

inline const TableItemExpr *
ExprMaker::make_table_item_expr(
    const Expr *const lvalue,
    const Expr *const index,
    const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(
        !!lvalue,
        !!index,
        !SemUtils::is_rvalue_expr(lvalue),
        SemUtils::is_expr_with_symbol(lvalue)
    );
    const auto expr_w_symbol = static_cast<const ExprWSymbol *>(REQUIRE_PTR(lvalue));
    const auto lvalue_symbol = static_cast<const Symbol *>(expr_w_symbol->symbol);
    const auto table_item_expr = new const TableItemExpr(
        expr_loc,
        lvalue_symbol,
        index);
    expr_sink_.push_back(table_item_expr);
    return table_item_expr;
}

inline const VariableExpr *
ExprMaker::make_variable_expr(const Symbol *const symbol, const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!symbol);
    const auto variable_expr = new const VariableExpr(expr_loc, symbol);
    expr_sink_.push_back(variable_expr);
    return variable_expr;
}
} // namespace Alpha
#endif //ALPHA_EXPR_MAKER_HPP
