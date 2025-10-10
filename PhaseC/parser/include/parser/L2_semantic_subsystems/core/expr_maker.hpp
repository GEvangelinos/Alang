#ifndef ALPHA_EXPR_MAKER_HPP
#define ALPHA_EXPR_MAKER_HPP

#include "ir_expr.hpp"
#include "semantic_utils.hpp"
#include "core/basics.hpp"
#include "core/source_location.hpp"
#include "parser/parser_context.hpp"
#include "support/misc_tools.hpp"

namespace alpha
{
class ExprMaker : private Immobile
{
    friend class SemanticSystem; // We friend our master
public:
    explicit ExprMaker(ParseCtx *parse_ctx);
    ~ExprMaker() noexcept;

    template<bool make_dup = false>
    [[nodiscard]] const ArithmeticExpr *make_arithmetic_expr(
        SourceLocation expr_loc, const ArithmeticExpr *dup_from = nullptr);
    [[nodiscard]] const ArithmeticExpr *make_arithmetic_expr(
        SourceLocation expr_loc, const VarSymbol *var_symbol);
    template<bool make_dup = false>
    [[nodiscard]] const BoolExpr *make_bool_expr(
        SourceLocation expr_loc, const BoolExpr *dup_from = nullptr);
    [[nodiscard]] const BoolExpr *make_bool_expr(
        SourceLocation expr_loc, const VarSymbol *var_symbol);
    template<bool make_dup = false>
    [[nodiscard]] const NewTableExpr *make_new_table_expr(
        SourceLocation expr_loc, const NewTableExpr *dup_from = nullptr);
    [[nodiscard]] const NewTableExpr *make_new_table_expr(
        SourceLocation expr_loc, const VarSymbol *var_symbol);
    [[nodiscard]] const AssignExpr *make_assign_expr(
        SourceLocation expr_loc, const VarSymbol *var_symbol);
    [[nodiscard]] const ConstBoolExpr *make_const_bool_expr(
        SourceLocation expr_loc, bool bool_value);
    [[nodiscard]] const ConstIntExpr *make_const_int_expr(
        SourceLocation expr_loc, AlphaInt int_value);
    [[nodiscard]] const ConstFloatExpr *make_const_float_expr(
        SourceLocation expr_loc, AlphaFloat float_value);
    [[nodiscard]] const ConstStringExpr *make_const_string_expr(
        SourceLocation expr_loc, const char *str_value);
    [[nodiscard]] const ConstNilExpr *make_nil_expr(SourceLocation expr_loc);
    [[nodiscard]] const LibFuncExpr *make_lib_func_expr(
        SourceLocation expr_loc, const LibFuncSymbol *func_symbol);
    [[nodiscard]] const ProgFuncExpr *make_prog_func_expr(
        SourceLocation expr_loc, const ProgFuncSymbol *func_symbol);
    [[nodiscard]] const TableItemExpr *make_table_item_expr(
        SourceLocation expr_loc, const Expr *base, const Expr *index);
    [[nodiscard]] const VariableExpr *make_variable_expr(
        SourceLocation expr_loc, const VarSymbol *var_symbol);

    [[nodiscard]] const Expr *clone_with_updated_location(
        SourceLocation new_loc, const Expr *donor_expr);

private:
    ParseCtx *const parse_ctx_;
    std::vector<const Expr *> expr_sink_;
};

#define DEFINE_MAKER_WITH_TEMP_SYMBOL(EXPR_TYPE, FN_NAME)                              \
    template<bool make_dup>                                                            \
    const EXPR_TYPE *                                                                  \
    ExprMaker::FN_NAME(const SourceLocation expr_loc, const EXPR_TYPE *const dup_from) \
    {                                                                                  \
        const EXPR_TYPE *expr = nullptr;                                               \
        if constexpr (make_dup)                                                        \
            expr = new const EXPR_TYPE(expr_loc, DEBUG_REQUIRE_PTR(dup_from)->var_symbol);   \
        else                                                                           \
            expr = new const EXPR_TYPE(expr_loc, parse_ctx_->new_temp());              \
        expr_sink_.push_back(expr);                                                    \
        return expr;                                                                   \
    }
DEFINE_MAKER_WITH_TEMP_SYMBOL(ArithmeticExpr, make_arithmetic_expr)
DEFINE_MAKER_WITH_TEMP_SYMBOL(BoolExpr, make_bool_expr)
DEFINE_MAKER_WITH_TEMP_SYMBOL(NewTableExpr, make_new_table_expr)
#undef DEFINE_MAKER_WITH_TEMP_SYMBOL

inline const ArithmeticExpr *
ExprMaker::make_arithmetic_expr(
    const SourceLocation expr_loc,
    const VarSymbol *const var_symbol)
{
    DEBUG_SMART_ASSERT(!!var_symbol);
    const auto *const arithmetic_expr = new const ArithmeticExpr(expr_loc, var_symbol);
    expr_sink_.push_back(arithmetic_expr);
    return arithmetic_expr;
}

inline const BoolExpr *
ExprMaker::make_bool_expr(
    const SourceLocation expr_loc,
    const VarSymbol *const var_symbol)
{
    DEBUG_SMART_ASSERT(!!var_symbol);
    const auto *const bool_expr = new const BoolExpr(expr_loc, var_symbol);
    expr_sink_.push_back(bool_expr);
    return bool_expr;
}

inline const NewTableExpr *
ExprMaker::make_new_table_expr(
    const SourceLocation expr_loc,
    const VarSymbol *const var_symbol)
{
    DEBUG_SMART_ASSERT(!!var_symbol);
    const auto *const new_table_expr = new const NewTableExpr(expr_loc, var_symbol);
    expr_sink_.push_back(new_table_expr);
    return new_table_expr;
}

inline const AssignExpr *
ExprMaker::make_assign_expr(
    const SourceLocation expr_loc,
    const VarSymbol *const var_symbol)
{
    DEBUG_SMART_ASSERT(!!var_symbol);
    const auto *const assign_expr = new const AssignExpr(expr_loc, var_symbol);
    expr_sink_.push_back(assign_expr);
    return assign_expr;
}

inline const ConstBoolExpr *
ExprMaker::make_const_bool_expr(const SourceLocation expr_loc, const bool bool_value)
{
    const auto *const const_bool_expr = new const ConstBoolExpr(expr_loc, bool_value);
    expr_sink_.push_back(const_bool_expr);
    return const_bool_expr;
}

inline const ConstIntExpr *
ExprMaker::make_const_int_expr(const SourceLocation expr_loc, const AlphaInt int_value)
{
    const auto *const const_int_expr = new const ConstIntExpr(expr_loc, int_value);
    expr_sink_.push_back(const_int_expr);
    return const_int_expr;
}

inline const ConstFloatExpr *
ExprMaker::make_const_float_expr(const SourceLocation expr_loc, const AlphaFloat float_value)
{
    const auto *const const_float_expr = new const ConstFloatExpr(expr_loc, float_value);
    expr_sink_.push_back(const_float_expr);
    return const_float_expr;
}

inline const ConstStringExpr *
ExprMaker::make_const_string_expr(const SourceLocation expr_loc, const char *const str_value)
{
    DEBUG_SMART_ASSERT(!!str_value);
    const auto *const const_str_expr = new const ConstStringExpr(expr_loc, str_value);
    expr_sink_.push_back(const_str_expr);
    return const_str_expr;
}

inline const ConstNilExpr *
ExprMaker::make_nil_expr(const SourceLocation expr_loc)
{
    const auto *const nil_expr = new const ConstNilExpr(expr_loc);
    expr_sink_.push_back(nil_expr);
    return nil_expr;
}

inline const LibFuncExpr *
ExprMaker::make_lib_func_expr(const SourceLocation expr_loc, const LibFuncSymbol *const func_symbol)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    const auto *const lib_func_expr = new const LibFuncExpr(expr_loc, func_symbol);
    expr_sink_.push_back(lib_func_expr);
    return lib_func_expr;
}

inline const ProgFuncExpr *
ExprMaker::make_prog_func_expr(
    const SourceLocation expr_loc,
    const ProgFuncSymbol *const func_symbol)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    const auto *const prog_func_expr = new const ProgFuncExpr(expr_loc, func_symbol);
    expr_sink_.push_back(prog_func_expr);
    return prog_func_expr;
}

inline const TableItemExpr *
ExprMaker::make_table_item_expr(
    const SourceLocation expr_loc,
    const Expr *const base,
    const Expr *const index)
{
    DEBUG_SMART_ASSERT(
        !!base, !!index,
        base->is_lvalue(),
        base->has_symbol()
    );
    // TODO POLISH
    DEBUG_SMART_ASSERT(
        base->type == Expr::Type::VARIABLE &&
        "if it fails then we must check for lib and prog, assign , tableitem "
    );
    const auto *const expr_w_var_symbol = static_cast<const ExprWVarSymbol *>(base);
    const auto *const lvalue_symbol = static_cast<const VarSymbol *>(expr_w_var_symbol->var_symbol);
    const auto *const table_item_expr = new const TableItemExpr(expr_loc, lvalue_symbol, index);
    expr_sink_.push_back(table_item_expr);
    return table_item_expr;
}

inline const VariableExpr *
ExprMaker::make_variable_expr(const SourceLocation expr_loc, const VarSymbol *const var_symbol)
{
    DEBUG_SMART_ASSERT(!!var_symbol);
    const auto *const variable_expr = new const VariableExpr(expr_loc, var_symbol);
    expr_sink_.push_back(variable_expr);
    return variable_expr;
}

} // namespace alpha
#endif //ALPHA_EXPR_MAKER_HPP
