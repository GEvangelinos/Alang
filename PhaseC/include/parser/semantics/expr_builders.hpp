#ifndef EXPR_BUILDERS_HPP
#define EXPR_BUILDERS_HPP

#include "expr_folder.hpp"
#include "expr_maker.hpp"
#include "quad_handler.hpp"
#include "semantic_utils.hpp"
#include "core/alpha_core_types.hpp"

namespace Alpha
{
class ConstBuilder
{
public:
    explicit ConstBuilder(ExprMaker *const expr_maker)
        : expr_maker_(Utils::require_ptr(expr_maker)) {}

    [[nodiscard]] const Expr *build_true_expr(SourceLocation loc);
    [[nodiscard]] const Expr *build_false_expr(SourceLocation loc);
    [[nodiscard]] const Expr *build_int_expr(AlphaInt value, SourceLocation loc);
    [[nodiscard]] const Expr *build_float_expr(AlphaFloat value, SourceLocation loc);
    [[nodiscard]] const Expr *build_string_expr(const char *value, SourceLocation loc);
    [[nodiscard]] const Expr *build_nil_expr(SourceLocation loc);

private:
    ExprMaker *const expr_maker_;
};

class BasicBuilder
{
public:
    struct Options
    {
        bool fold_arithmetic;
        bool fold_relational;
        bool fold_logical;
    };

    BasicBuilder(Options &&options, ExprSnitch *snitch, ExprMaker *expr_maker,
                 ExprFolder *expr_folder, QuadHandler *quad_handler, ParseCache *parse_cache);

    [[nodiscard]] const Expr *build_arithmetic(
        IOPCode iopc, const Expr *lhs, const Expr *rhs,
        SourceLocation lhs_loc, SourceLocation rhs_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_uminus(
        const Expr *expr, SourceLocation term_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_relational(
        IOPCode iopc, const Expr *lhs, const Expr *rhs,
        SourceLocation lhs_loc, SourceLocation rhs_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_or(
        const Expr *lhs, const Expr *rhs,
        SourceLocation lhs_loc, SourceLocation rhs_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_and(
        const Expr *lhs, const Expr *rhs,
        SourceLocation lhs_loc, SourceLocation rhs_loc, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_not(const Expr *expr, SourceLocation result_loc);


    inline Expr *SemanticBuilder::resolve_assign_expr(Expr *lvalue, Expr *expr,
                                                      const SourceLocation assign_loc)
    {
        DEBUG_SMART_ASSERT(!!lvalue, !!expr);

        validate_lvalue_for_assignment(lvalue->symbol, assign_loc);
        if(lvalue->type == Expr::Type::TABLE_ITEM)
            return handle_table_item_assignment(lvalue, expr, assign_loc);
        return handle_direct_assignment(lvalue, expr, assign_loc);
    }

    inline Expr *SemanticBuilder::handle_table_item_assignment(Expr *lvalue, Expr *expr,
                                                               SourceLocation assign_loc)
    {
        parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, lvalue, lvalue->index, expr,
                                          assign_loc);

        Expr *rvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        return parse_ctx_.expr_handler.make_expr_assign(rvalue, assign_loc);
    }

    inline Expr *SemanticBuilder::handle_direct_assignment(Expr *lvalue, Expr *expr,
                                                           SourceLocation assign_loc)
    {
        parse_ctx_.quad_handler.emit_quad(
                IOPCode::ASSIGN, expr, nullptr, lvalue,
                assign_loc);
        // TODO (NOT IMPORTANT): loc (can we construct it from expr (to catch
        // whole assignment expression?))

        Expr *assignExpr = parse_ctx_.expr_handler.make_expr_assign(parse_ctx_.new_temp(),
                assign_loc //
        );

        parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, lvalue, nullptr, assignExpr,
                                          k_no_location);

        return assignExpr;
    }

private:
    const Options options_;
    ExprSnitch *const snitch_;
    ExprMaker *const expr_maker_;
    ExprFolder *const expr_folder_;
    QuadHandler *const quad_handler_;
    ParseCache *const parse_cache_;

    const Expr *convert_to_bool_form(const Expr *expr, SourceLocation expr_loc);
};

inline const Expr *
ConstBuilder::build_true_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(true, loc);
}

inline const Expr *
ConstBuilder::build_false_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(false, loc);
}

inline const Expr *
ConstBuilder::build_int_expr(const AlphaInt value, const SourceLocation loc)
{
    return expr_maker_->make_const_int_expr(value, loc);
}

inline const Expr *
ConstBuilder::build_float_expr(const AlphaFloat value, const SourceLocation loc)
{
    return expr_maker_->make_const_float_expr(value, loc);
}

inline const Expr *
ConstBuilder::build_string_expr(const char *const value, const SourceLocation loc)
{
    return expr_maker_->make_const_string_expr(value, loc);
}

inline const Expr *
ConstBuilder::build_nil_expr(const SourceLocation loc)
{
    return expr_maker_->make_nil_expr(loc);
}

inline BasicBuilder::BasicBuilder(
    Options &&options,
    ExprSnitch *const snitch,
    ExprMaker *const expr_maker,
    ExprFolder *const expr_folder,
    QuadHandler *const quad_handler,
    ParseCache *const parse_cache)
    : options_(std::move(options)),
      snitch_(Utils::require_ptr(snitch)),
      expr_maker_(Utils::require_ptr(expr_maker)),
      expr_folder_(Utils::require_ptr(expr_folder)),
      quad_handler_(Utils::require_ptr(quad_handler)),
      parse_cache_(Utils::require_ptr(parse_cache)) {}

inline const Expr *
BasicBuilder::build_arithmetic(
    const IOPCode iopc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation lhs_loc,
    const SourceLocation rhs_loc,
    const SourceLocation result_loc)

{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    snitch_->report_if_not_arithmetic(iopc, lhs, lhs_loc, OperandSide::LEFT);
    snitch_->report_if_not_arithmetic(iopc, rhs, rhs_loc, OperandSide::RIGHT);
    std::cout << "YES\n";

    if (options_.fold_arithmetic &&
        SemUtils::is_const_arithmetic_expr(lhs) &&
        SemUtils::is_const_arithmetic_expr(rhs))
        return expr_folder_->fold_arithmetic(iopc, lhs, rhs, result_loc);
    std::cout << "no\n";


    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next_quad(iopc, lhs, rhs, arithmetic_expr, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::build_uminus(
    const Expr *const expr,
    const SourceLocation term_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    snitch_->report_if_not_arithmetic(IOPCode::UMINUS, expr, term_loc, OperandSide::UNARY);

    if (options_.fold_arithmetic && SemUtils::is_const_arithmetic_expr(expr))
        return expr_folder_->fold_uminus(expr, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(term_loc);
    quad_handler_->emit_next_quad(IOPCode::UMINUS, expr, nullptr, arithmetic_expr, term_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::build_relational(
    const IOPCode iopc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation lhs_loc,
    const SourceLocation rhs_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    snitch_->report_if_not_relational(iopc, lhs, lhs_loc, OperandSide::LEFT);
    snitch_->report_if_not_relational(iopc, rhs, rhs_loc, OperandSide::RIGHT);

    if (options_.fold_relational &&
        SemUtils::is_relational_equality_iopcode(iopc) &&
        SemUtils::is_rvalue_expr(lhs) &&
        SemUtils::is_rvalue_expr(rhs))
        return expr_folder_->fold_relational_equality(iopc, lhs, rhs);
    if (options_.fold_relational &&
        SemUtils::is_relational_arithmetic_iopcode(iopc) &&
        SemUtils::is_const_arithmetic_expr(lhs) &&
        SemUtils::is_const_arithmetic_expr(rhs))
        return expr_folder_->fold_relational_arithmetic(iopc, lhs, rhs);

    BoolExpr *result_expr = expr_maker_->make_bool_expr(result_loc);

    result_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(iopc, lhs, rhs, nullptr, result_loc);
    result_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(IOPCode::JUMP, nullptr, nullptr, nullptr, result_loc);
    return result_expr;
}

inline const Expr *
BasicBuilder::build_logical_or(
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation lhs_loc,
    const SourceLocation rhs_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    lhs = convert_to_bool_form(lhs, lhs_loc);
    rhs = convert_to_bool_form(rhs, rhs_loc);

    if (options_.fold_logical)
        if (SemUtils::is_const_bool_expr(lhs) || SemUtils::is_const_bool_expr(rhs))
            return expr_folder_->fold_logical_or(lhs, rhs);

    DEBUG_SMART_ASSERT(lhs->type == Expr::Type::BOOL_EXPR && rhs->type == Expr::Type::BOOL_EXPR);

    BoolExpr *bool_result_expr = expr_maker_->make_bool_expr(result_loc);

    BoolExpr *const left_bool = static_cast<BoolExpr *>(const_cast<Expr *>(lhs));
    BoolExpr *const right_bool = static_cast<BoolExpr *>(const_cast<Expr *>(rhs));

    DEBUG_SMART_ASSERT(!parse_cache_->short_circuit_jump_stack.empty());
    for (const LabelID quad_label : left_bool->false_list)
        quad_handler_->patch_quad(quad_label, parse_cache_->short_circuit_jump_stack.top());
    parse_cache_->short_circuit_jump_stack.pop();
    left_bool->false_list.clear();

    bool_result_expr->true_list.reserve(left_bool->true_list.size() + right_bool->true_list.size());
    bool_result_expr->true_list.insert(bool_result_expr->true_list.end(),
                                       left_bool->true_list.begin(),
                                       left_bool->true_list.end());
    bool_result_expr->true_list.insert(bool_result_expr->true_list.end(),
                                       right_bool->true_list.begin(),
                                       right_bool->true_list.end());
    bool_result_expr->false_list = right_bool->false_list;
    return bool_result_expr;
}

//
// inline const Expr *
// BasicBuilder::build_logical_and(
//     const Expr *left, const Expr *right,
//     SourceLocation left_loc, SourceLocation right_loc, SourceLocation result_loc)
// {
//     // Check your solution on GitHub (latest commit on branch feature/ir-gen) (23/05/2025)
//     UNIMPLEMENTED();
// }
//
// inline const Expr *
// BasicBuilder::build_logical_not(const Expr *expr, SourceLocation result_loc)
// {
//     // Check your solution on GitHub (latest commit on branch feature/ir-gen) (23/05/2025)
//     UNIMPLEMENTED();
// }

inline const Expr *
BasicBuilder::convert_to_bool_form(const Expr *const expr, const SourceLocation expr_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type == Expr::Type::BOOL_EXPR)
        return expr;
    if (SemUtils::is_rvalue_expr(expr))
        return SemUtils::as_bool(expr) ? expr_maker_->premade_true : expr_maker_->premade_false;

    BoolExpr *const bool_expr = expr_maker_->make_bool_expr(expr_loc);
    bool_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(
        IOPCode::IF_EQ, expr, expr_maker_->premade_true, nullptr, expr_loc);
    bool_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(IOPCode::JUMP, nullptr, nullptr, nullptr, expr_loc);
    return bool_expr;
}
} // namespace Alpha
#endif // EXPR_BUILDERS_HPP
