#ifndef ALPHA_EXPR_MAKER_HPP
#define ALPHA_EXPR_MAKER_HPP

#include "ir_expr.hpp"
#include "semantic_utils.hpp"
#include "core/basics.hpp"
#include "core/source_location.hpp"
#include "parser/parser_context.hpp"

namespace alpha
{
class ExprMaker : private Immobile
{
public:
    explicit ExprMaker(ParseCtx *parse_ctx);
    ~ExprMaker() noexcept;

    template<bool make_dup = false>
    [[nodiscard]] const ArithmeticExpr *make_arithmetic_expr(
        SourceLocation expr_loc, const ArithmeticExpr *dup_from = nullptr);
    template<bool make_dup = false>
    [[nodiscard]] const BoolExpr *make_bool_expr(
        SourceLocation expr_loc, const BoolExpr *dup_from = nullptr);
    template<bool make_dup = false>
    [[nodiscard]] const NewTableExpr *make_new_table_expr(
        SourceLocation expr_loc, const NewTableExpr *dup_from = nullptr);
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
        SourceLocation expr_loc, const FuncSymbol *func_symbol);
    [[nodiscard]] const ProgFuncExpr *make_prog_func_expr(
        SourceLocation expr_loc, const FuncSymbol *func_symbol);
    [[nodiscard]] const TableItemExpr *make_table_item_expr(
        SourceLocation expr_loc, const Expr *lvalue, const Expr *index);
    [[nodiscard]] const VariableExpr *make_variable_expr(
        SourceLocation expr_loc, const VarSymbol *var);
    [[nodiscard]] const Expr *clone_with_updated_location(
        SourceLocation new_loc, const Expr *donor_Expr);

private:
    ParseCtx *const parse_ctx_;
    std::vector<const Expr *> expr_sink_;
};

inline
ExprMaker::ExprMaker(ParseCtx *const parse_ctx)
    : parse_ctx_(utils::require_ptr(parse_ctx)) {}

inline ExprMaker::~ExprMaker() noexcept
{
    for (const Expr *e: expr_sink_)
    {
        DEBUG_SMART_ASSERT(!!e);
        using ET = Expr::Type;
        switch (e->type)
        {
        // clang-format off
        case ET::ARITHMETIC_EXPR: delete static_cast<const ArithmeticExpr *>(e); break;
        case ET::ASSIGN_EXPR: delete static_cast<const AssignExpr *>(e);         break;
        case ET::BOOL_EXPR: delete static_cast<const BoolExpr *>(e);             break;
        case ET::CONST_BOOL: delete static_cast<const ConstBoolExpr *>(e);       break;
        case ET::CONST_INT: delete static_cast<const ConstIntExpr *>(e);         break;
        case ET::CONST_FLOAT: delete static_cast<const ConstFloatExpr *>(e);     break;
        case ET::CONST_STRING: delete static_cast<const ConstStringExpr *>(e);   break;
        case ET::CONST_NIL: delete static_cast<const ConstNilExpr *>(e);         break;
        case ET::LIBRARY_FUNCTION: delete static_cast<const LibFuncExpr *>(e);   break;
        case ET::PROGRAM_FUNCTION: delete static_cast<const ProgFuncExpr *>(e);  break;
        case ET::NEW_TABLE: delete static_cast<const NewTableExpr *>(e);         break;
        case ET::TABLE_ITEM: delete static_cast<const TableItemExpr *>(e);       break;
        case ET::VARIABLE: delete static_cast<const VariableExpr *>(e);          break;
        // clang-format on
        default: UNREACHABLE(FMT::format(
                "Unknown Expr::Type. Expr::Type's int value = {}", static_cast<int>(e->type)));
        }
    }
}

#define DEFINE_MAKER_WITH_TEMP_SYMBOL(EXPR_TYPE, FN_NAME)                              \
    template<bool make_dup>                                                            \
    const EXPR_TYPE *                                                                  \
    ExprMaker::FN_NAME(const SourceLocation expr_loc, const EXPR_TYPE *const dup_from) \
    {                                                                                  \
        const EXPR_TYPE *expr = nullptr;                                               \
        if constexpr (make_dup)                                                        \
            expr = new const EXPR_TYPE(expr_loc, REQUIRE_PTR(dup_from)->var_symbol);   \
        else                                                                           \
            expr = new const EXPR_TYPE(expr_loc, parse_ctx_->new_temp());              \
        expr_sink_.push_back(expr);                                                    \
        return expr;                                                                   \
    }
DEFINE_MAKER_WITH_TEMP_SYMBOL(ArithmeticExpr, make_arithmetic_expr)
DEFINE_MAKER_WITH_TEMP_SYMBOL(BoolExpr, make_bool_expr)
DEFINE_MAKER_WITH_TEMP_SYMBOL(NewTableExpr, make_new_table_expr)
#undef DEFINE_MAKER_WITH_TEMP_SYMBOL

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
ExprMaker::make_lib_func_expr(const SourceLocation expr_loc, const FuncSymbol *const func_symbol)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    const auto *const lib_func_expr = new const LibFuncExpr(expr_loc, func_symbol);
    expr_sink_.push_back(lib_func_expr);
    return lib_func_expr;
}

inline const ProgFuncExpr *
ExprMaker::make_prog_func_expr(
    const SourceLocation expr_loc,
    const FuncSymbol *const func_symbol)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    const auto *const prog_func_expr = new const ProgFuncExpr(expr_loc, func_symbol);
    expr_sink_.push_back(prog_func_expr);
    return prog_func_expr;
}

inline const TableItemExpr *
ExprMaker::make_table_item_expr(
    const SourceLocation expr_loc,
    const Expr *const lvalue,
    const Expr *const index)
{
    DEBUG_SMART_ASSERT(
        !!lvalue, !!index,
        SemUtils::is_lvalue_expr(lvalue),
        SemUtils::is_expr_with_symbol(lvalue)
    );
    // TODO POLISH
    DEBUG_SMART_ASSERT(
        lvalue->type == Expr::Type::VARIABLE &&
        "if it fails then we must check for lib and prog, assign , tableitem ");
    const auto *const expr_w_var_symbol = static_cast<const ExprWVarSymbol *>(REQUIRE_PTR(lvalue));
    const auto *const lvalue_symbol = static_cast<const VarSymbol *>(expr_w_var_symbol->var_symbol);
    const auto *const table_item_expr = new const TableItemExpr(expr_loc, lvalue_symbol, index);
    expr_sink_.push_back(table_item_expr);
    return table_item_expr;
}

inline const VariableExpr *
ExprMaker::make_variable_expr(const SourceLocation expr_loc, const VarSymbol *const var)
{
    DEBUG_SMART_ASSERT(!!var);
    const auto *const variable_expr = new const VariableExpr(expr_loc, var);
    expr_sink_.push_back(variable_expr);
    return variable_expr;
}

inline const Expr *ExprMaker::clone_with_updated_location(
    const SourceLocation new_loc,
    const Expr *const donor_Expr)
{
    DEBUG_SMART_ASSERT(!!donor_Expr);
    using ET = Expr::Type;
    switch (donor_Expr->type)
    {
    case ET::ARITHMETIC_EXPR:
        return make_arithmetic_expr<true>(new_loc, static_cast<const ArithmeticExpr *>(donor_Expr));
    case ET::ASSIGN_EXPR:
        return make_assign_expr(new_loc, static_cast<const AssignExpr *>(donor_Expr)->var_symbol);
    case ET::BOOL_EXPR:
        return make_bool_expr<true>(new_loc, static_cast<const BoolExpr *>(donor_Expr));
    case ET::CONST_BOOL:
        return make_const_bool_expr(new_loc, static_cast<const ConstBoolExpr *>(donor_Expr)->value);
    case ET::CONST_INT:
        return make_const_int_expr(new_loc, static_cast<const ConstIntExpr *>(donor_Expr)->value);
    case ET::CONST_FLOAT:
        return make_const_float_expr(
            new_loc, static_cast<const ConstFloatExpr *>(donor_Expr)->value);
    case ET::CONST_STRING:
        return make_const_string_expr(
            new_loc, static_cast<const ConstStringExpr *>(donor_Expr)->value);
    case ET::CONST_NIL:
        return make_nil_expr(new_loc);
    case ET::LIBRARY_FUNCTION:
        return make_lib_func_expr(
            new_loc, static_cast<const LibFuncExpr *>(donor_Expr)->func_symbol);
    case ET::PROGRAM_FUNCTION:
        return make_prog_func_expr(
            new_loc, static_cast<const ProgFuncExpr *>(donor_Expr)->func_symbol);
    case ET::NEW_TABLE:
        return make_new_table_expr<true>(new_loc, static_cast<const NewTableExpr *>(donor_Expr));
    case ET::TABLE_ITEM:
    {
        const Expr *index = static_cast<const TableItemExpr *>(donor_Expr)->index;
        return make_table_item_expr(new_loc, donor_Expr, index);
    }
    case ET::VARIABLE:
        return make_variable_expr(
            new_loc, static_cast<const VariableExpr *>(donor_Expr)->var_symbol);
    default:
        UNREACHABLE(FMT::format(
            "Unknown Expr::Type. Expr::Type's int value = {}", static_cast<int>(donor_Expr->type)));
    }
}
} // namespace alpha
#endif //ALPHA_EXPR_MAKER_HPP
