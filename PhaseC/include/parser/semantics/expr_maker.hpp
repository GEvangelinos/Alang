// TODO: make const_bool_true, const_bool_false, const_int = 1 static expressions and reusable.
// Also you should you should be able to modify their location, so QuadHandler can take their latest
// location of use, and append it to the quad it is making.

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
class ExprMaker : private Immobile
{
public:
    explicit ExprMaker(SemanticDriver *sd);
    ~ExprMaker() noexcept;

    [[nodiscard]] Expr *make_arithmetic_expr(IOPCode iopc, const Expr *left, const Expr *right,
                                             SourceLocation left_loc, SourceLocation right_loc,
                                             SourceLocation expr_loc);
    [[nodiscard]] Expr *make_assign_expr(const Expr *rvalue, SourceLocation expr_loc);
    [[nodiscard]] Expr *make_bool_expr(SourceLocation expr_loc);
    [[nodiscard]] Expr *make_const_bool_expr(bool bool_value, SourceLocation expr_loc);
    [[nodiscard]] Expr *make_const_int_expr(i64 int_value, SourceLocation expr_loc);
    [[nodiscard]] Expr *make_const_float_expr(f64 float_value, SourceLocation expr_loc);
    [[nodiscard]] Expr *make_const_string_expr(const char *str_value, SourceLocation expr_loc);
    [[nodiscard]] Expr *make_const_nil_expr(SourceLocation expr_loc);
    [[nodiscard]] Expr *make_new_table_expr(SourceLocation expr_loc);
    [[nodiscard]] Expr *make_program_function_expr(const Function *function_symbol);
    [[nodiscard]] Expr *make_table_item_expr(const Expr *lvalue, const Expr *index,
                                             SourceLocation expr_loc);
    [[nodiscard]] Expr *make_expr_variable(const Symbol *symbol, SourceLocation expr_loc);

private:
    SemanticDriver *const sd_;
    ParseCtx *const &parse_ctx_;
    std::vector<const Expr *> expr_sink_;
};

inline
ExprMaker::ExprMaker(SemanticDriver *const sd)
    : sd_(Utils::require_ptr(sd)),
      parse_ctx_(Utils::require_ptr(Utils::require_ptr(sd)->parse_ctx_)) {}

inline ExprMaker::~ExprMaker() noexcept
{
    for(const Expr *e : expr_sink_)
    {
        DEBUG_SMART_ASSERT(!!e);
        if(e->type == Expr::Type::CONST_STRING)
            delete[] e->const_str;
        delete e;
    }
}

inline Expr *
ExprMaker::make_arithmetic_expr(
    const IOPCode iopc,
    const Expr *const left,
    const Expr *const right,
    const SourceLocation left_loc,
    const SourceLocation right_loc,
    const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!left, !!right);
    sd_->expr_validator_.report_if_not_arithmetic(iopc, left, left_loc, OperandSide::LEFT);
    sd_->expr_validator_.report_if_not_arithmetic(iopc, right, right_loc, OperandSide::RIGHT);

    if (sd_->sem_opts_.fold_arithmetic)
        if (SemUtils::is_const_numeric_expr(left) && SemUtils::is_const_numeric_expr(right))
            if (Expr *folded = sd_->expr_folder_.fold_arithmetic(iopc, left, right, expr_loc))
                return folded;

    Expr *arithmetic_expr = new Expr{
        .type = Expr::Type::ARITHMETIC_EXPR,
        .symbol = parse_ctx_->new_temp(),
        .location = expr_loc,
        .union_control = nullptr,
    };

    expr_sink_.push_back(arithmetic_expr);
    sd_->quad_handler.emit_next_quad(iopc, left, right, arithmetic_expr, expr_loc);
    return arithmetic_expr;
}

inline Expr *
ExprMaker::make_assign_expr(const Expr *const rvalue, const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!rvalue);
    Expr *assign_expr = new Expr{
        .type = Expr::Type::ASSIGN_EXPR,
        .symbol = rvalue->symbol,
        .location = expr_loc,
        .index = rvalue->index,
    };
    expr_sink_.push_back(assign_expr);
    return assign_expr;
}

inline Expr *ExprMaker::make_bool_expr(const SourceLocation expr_loc)
{
    Expr *bool_expr = new Expr{
        .type = Expr::Type::BOOLEAN_EXPR,
        .symbol = parse_ctx_->new_temp(),
        .location = expr_loc,
        .union_control = nullptr,
        .backpatch_lists = new BackpatchLists{},
    };
    expr_sink_.push_back(bool_expr);
    return bool_expr;
}

inline Expr *
ExprMaker::make_const_bool_expr(const bool bool_value, const SourceLocation expr_loc)
{
    Expr *bool_expr = new Expr{
        .type = Expr::Type::CONST_BOOL,
        .symbol = nullptr,
        .location = expr_loc,
        .const_bool = bool_value,
    };
    expr_sink_.push_back(bool_expr);
    return bool_expr;
}

inline Expr *
ExprMaker::make_const_int_expr(const i64 int_value, const SourceLocation expr_loc)
{
    Expr *int_expr = new Expr{
        .type = Expr::Type::CONST_INT,
        .symbol = nullptr,
        .location = expr_loc,
        .const_int = int_value,
    };
    expr_sink_.push_back(int_expr);
    return int_expr;
}

inline Expr *
ExprMaker::make_const_float_expr(const f64 float_value, const SourceLocation expr_loc)
{
    Expr *float_expr = new Expr{
        .type = Expr::Type::CONST_FLOAT,
        .symbol = nullptr,
        .location = expr_loc,
        .const_real = float_value,
    };
    expr_sink_.push_back(float_expr);
    return float_expr;
}

inline Expr *
ExprMaker::make_const_string_expr(const char *const str_value, const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!str_value);
    Expr *str_expr = new Expr{
        .type = Expr::Type::CONST_STRING,
        .symbol = nullptr,
        .location = expr_loc,
        .const_str = Utils::cstrdup(str_value),
    };
    expr_sink_.push_back(str_expr);
    return str_expr;
}

inline Expr *
ExprMaker::make_const_nil_expr(const SourceLocation expr_loc)
{
    Expr *nil_expr = new Expr{
        .type = Expr::Type::CONST_NIL,
        .symbol = nullptr,
        .location = expr_loc,
        .union_control = nullptr,
    };
    expr_sink_.push_back(nil_expr);
    return nil_expr;
}

inline Expr *ExprMaker::make_new_table_expr(const SourceLocation expr_loc)
{
    Expr *new_table_expr = new Expr{
        .type = Expr::Type::NEW_TABLE,
        .symbol = parse_ctx_->new_temp(),
        .location = expr_loc,
        .union_control = nullptr,
    };
    expr_sink_.push_back(new_table_expr);
    return new_table_expr;
}

inline Expr *
ExprMaker::make_program_function_expr(const Function *const function_symbol)
{
    DEBUG_SMART_ASSERT(!!function_symbol);
    Expr *progfunc_expr = new Expr{
        .type = Expr::Type::PROGRAM_FUNCTION,
        .symbol = function_symbol,
        .location = function_symbol->location,
        .union_control = nullptr,
    };
    expr_sink_.push_back(progfunc_expr);
    return progfunc_expr;
}

inline Expr *
ExprMaker::make_table_item_expr(
    const Expr *const lvalue,
    const Expr *const index,
    const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!index);
    Expr *table_item_expr = new Expr{
        .type = Expr::Type::TABLE_ITEM,
        .symbol = lvalue->symbol,
        .location = expr_loc,
        .index = index,
    };
    expr_sink_.push_back(table_item_expr);
    return table_item_expr;
}

inline Expr *
ExprMaker::make_variable_expr(const Symbol *const symbol, const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!symbol);
    Expr *expr_lvalue = new Expr{
        .type = Expr::Type::VARIABLE,
        .symbol = symbol,
        .location = expr_loc,
        .union_control = nullptr,
    };
    expr_sink_.push_back(expr_lvalue);
    return expr_lvalue;
}
} // namespace Alpha
#endif //ALPHA_EXPR_MAKER_HPP
