#ifndef EXPR_BUILDERS_HPP
#define EXPR_BUILDERS_HPP

#include "diagnostics/diagnostic_engine.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include "L3_ir_infra/expr_folder.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/quad_handler.hpp"
#include "parser/ir.hpp"
#include "parser/semantic_utils.hpp"

namespace Alpha
{
class Backpatcher
{
private:
    ExprMaker *const expr_maker_ = nullptr;
    ParseCache *const parse_cache_ = nullptr;
    QuadHandler *const quad_handler_ = nullptr;

public:
    struct OrStrategy
    {
        static std::vector<LabelID> &backpatch_list(BoolExpr *expr) { return expr->false_list; }
        static std::vector<LabelID> &merge_lhs_list(BoolExpr *expr) { return expr->true_list; }
        static std::vector<LabelID> &merge_rhs_list(BoolExpr *expr) { return expr->true_list; }
        static std::vector<LabelID> &assign_list(BoolExpr *expr) { return expr->false_list; }
    };

    struct AndStrategy
    {
        static std::vector<LabelID> &backpatch_list(BoolExpr *expr) { return expr->true_list; }
        static std::vector<LabelID> &merge_lhs_list(BoolExpr *expr) { return expr->false_list; }
        static std::vector<LabelID> &merge_rhs_list(BoolExpr *expr) { return expr->false_list; }
        static std::vector<LabelID> &assign_list(BoolExpr *expr) { return expr->true_list; }
    };

    explicit Backpatcher(const SemanticSystemServices &services);

    template<typename Strategy>
    [[nodiscard]] const Expr *resolve_lazy_bool_expr(
        const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    void finalize_bool_expr(const Expr *expr);
};


class AssignBuilder
{
public:
    struct Options
    {
        bool record_constant_variables;
    };

    AssignBuilder(Options &&options, const SemanticSystemServices &services);

    [[nodiscard]] const Expr *build_assignment(
        const Expr *lvalue, const Expr *rvalue, SourceLocation result_loc);

private:
    const Options options_;
    DiagnosticReporter *const dr_;
    ParseCtx *const parse_ctx_;
    ExprMaker *const expr_maker_;
    ExprSnitch *const expr_snitch_;
    QuadHandler *const quad_handler_;
    SemanticSystemBridge *const ss_bridge_;

    void validate_lvalue_for_assignment(const Expr *lvalue, SourceLocation assign_loc);
    [[nodiscard]] const Expr *handle_table_item_assignment(
        const Expr *lvalue, const Expr *rvalue, SourceLocation result_loc);
    [[nodiscard]] const Expr *handle_direct_assignment(
        const Expr *lvalue, const Expr *rvalue, SourceLocation result_loc);
    [[nodiscard]] static bool try_record_const_value(const Expr *lvalue, const Expr *rvalue);
};

class BasicBuilder
{
public:
    struct Options
    {
        bool fold_arithmetic;
        bool fold_relational;
        bool fold_logical;
        bool propagate_constant_variable;
    };

    BasicBuilder(Options &&options, const SemanticSystemServices &services);

    [[nodiscard]] const Expr *build_uminus(
        const Expr *expr, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_arithmetic(
        IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_relational(
        IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_not(
        const Expr *expr, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_and(
        const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *build_logical_or(
        const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

private:
    const Options options_;
    DiagnosticReporter *const dr_;
    ExprMaker *const expr_maker_;
    ExprFolder *const expr_folder_;
    ExprSnitch *const snitch_;
    QuadHandler *const quad_handler_;
    Backpatcher *const backpatcher_;
};

class ConstBuilder
{
public:
    explicit ConstBuilder(const SemanticSystemServices &services);

    [[nodiscard]] const Expr *build_true_expr(SourceLocation loc);
    [[nodiscard]] const Expr *build_false_expr(SourceLocation loc);
    [[nodiscard]] const Expr *build_int_expr(AlphaInt value, SourceLocation loc);
    [[nodiscard]] const Expr *build_float_expr(AlphaFloat value, SourceLocation loc);
    [[nodiscard]] const Expr *build_string_expr(const char *value, SourceLocation loc);
    [[nodiscard]] const Expr *build_nil_expr(SourceLocation loc);

private:
    DiagnosticReporter *const dr_;
    ExprMaker *const expr_maker_;
};

inline
Backpatcher::Backpatcher(const SemanticSystemServices &services)
    : expr_maker_(REQUIRE_PTR(services.expr_maker)),
      parse_cache_(&REQUIRE_PTR(services.parse_ctx)->cache),
      quad_handler_(REQUIRE_PTR(services.quad_handler)) {}

template<typename Strategy>
[[nodiscard]] const Expr *
Backpatcher::resolve_lazy_bool_expr(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(lhs->type == Expr::Type::BOOL_EXPR && rhs->type == Expr::Type::BOOL_EXPR);
    BoolExpr *const left_bool = static_cast<BoolExpr *>(const_cast<Expr *>(lhs));
    BoolExpr *const right_bool = static_cast<BoolExpr *>(const_cast<Expr *>(rhs));
    BoolExpr *bool_result_expr = expr_maker_->make_bool_expr(result_loc);

    DEBUG_SMART_ASSERT(!parse_cache_->short_circuit_jump_stack.empty());
    for (const LabelID quad_label: Strategy::backpatch_list(left_bool))
        quad_handler_->patch_quad(quad_label, parse_cache_->short_circuit_jump_stack.top());
    parse_cache_->short_circuit_jump_stack.pop();
    Strategy::backpatch_list(left_bool).clear();

    auto &lhs_merge = Strategy::merge_lhs_list(left_bool);
    auto &rhs_merge = Strategy::merge_rhs_list(right_bool);
    auto &result_merge = Strategy::merge_lhs_list(bool_result_expr); // We could use merge_rhs too
    result_merge.reserve(lhs_merge.size() + rhs_merge.size());
    result_merge.insert(result_merge.end(), lhs_merge.begin(), lhs_merge.end());
    result_merge.insert(result_merge.end(), rhs_merge.begin(), rhs_merge.end());

    Strategy::assign_list(bool_result_expr) = Strategy::assign_list(right_bool);
    return bool_result_expr;
}

inline void
Backpatcher::finalize_bool_expr(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::BOOL_EXPR)
        return; // Nothing to backpatch.

    const BoolExpr *const bool_expr = static_cast<const BoolExpr *>(expr);
    auto *const qh = quad_handler_; // Alias for shorting names.

    DEBUG_SMART_ASSERT(!!bool_expr->symbol);

    qh->patch_list(bool_expr->true_list, qh->next_quad_label());
    qh->emit_next_quad(IOPCode::ASSIGN, expr_maker_->premade_true, nullptr, expr, expr->loc);
    qh->emit_next_quad(IOPCode::JUMP, nullptr, nullptr, nullptr, expr->loc, 2);
    quad_handler_->patch_list(bool_expr->false_list, quad_handler_->next_quad_label());
    qh->emit_next_quad(IOPCode::ASSIGN, expr_maker_->premade_false, nullptr, expr, expr->loc);
}

inline
AssignBuilder::AssignBuilder(Options &&options, const SemanticSystemServices &services)
    : options_(options),
      dr_(REQUIRE_PTR(services.dr)),
      parse_ctx_(REQUIRE_PTR(services.parse_ctx)),
      expr_maker_(REQUIRE_PTR(services.expr_maker)),
      expr_snitch_(REQUIRE_PTR(services.expr_snitch)),
      quad_handler_(REQUIRE_PTR(services.quad_handler)),
      ss_bridge_(REQUIRE_PTR(services.sd_bridge)) {}

inline const Expr *
AssignBuilder::build_assignment(
    const Expr *const lvalue,
    const Expr *const rvalue,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);
    validate_lvalue_for_assignment(lvalue, result_loc);
    if (lvalue->type == Expr::Type::TABLE_ITEM)
        return handle_table_item_assignment(lvalue, rvalue, result_loc);
    return handle_direct_assignment(lvalue, rvalue, result_loc);
}


inline void
AssignBuilder::validate_lvalue_for_assignment(
    const Expr *const lvalue,
    const SourceLocation assign_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue);
    if (!SemUtils::is_lvalue_expr(lvalue))
    {
        dr_->report_assign_lhs_not_lvalue(lvalue->type, lvalue->loc);
        return;
    }
    DEBUG_SMART_ASSERT(lvalue->has_symbol); // If here. Its Lvalue and all lvalues have symbols.
    const Symbol *const lv_symbol = static_cast<const ExprWSymbol *>(lvalue)->symbol;
    if (lv_symbol->type == Symbol::Type::LIBRARY_FUNCTION)
    {
        dr_->report_assign_to_libfunc(lv_symbol->name, assign_loc);
        return;
    }
    if (lv_symbol->type == Symbol::Type::PROGRAM_FUNCTION)
    {
        dr_->report_assign_to_func(lv_symbol->name, assign_loc, lv_symbol->loc);
        return;
    }
}

inline const Expr *
AssignBuilder::handle_table_item_assignment(
    const Expr *const lvalue,
    const Expr *const rvalue,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);
    DEBUG_SMART_ASSERT(lvalue->type == Expr::Type::TABLE_ITEM);
    const auto *const ti = static_cast<const TableItemExpr *>(lvalue);
    quad_handler_->emit_next_quad(IOPCode::TABLESETELEM, ti, ti->index, rvalue, result_loc);
    const Expr *ti_temp = ss_bridge_->emit_quad_if_table_item(lvalue);

    const Symbol *temp_symbol = static_cast<const TableItemExpr *>(ti_temp)->symbol;
    DEBUG_SMART_ASSERT(ti_temp->type == Expr::Type::TABLE_ITEM);
    return expr_maker_->make_assign_expr(temp_symbol, result_loc);
}

inline const Expr *
AssignBuilder::handle_direct_assignment(
    const Expr *const lvalue,
    const Expr *const rvalue,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);

    if (options_.record_constant_variables && try_record_const_value(lvalue, rvalue))
        return lvalue;

    // TODO: check todo 52 (on how to make this only when needed)
    const Expr *const temp = expr_maker_->make_assign_expr(parse_ctx_->new_temp(), result_loc);
    quad_handler_->emit_next_quad(IOPCode::ASSIGN, rvalue, nullptr, lvalue, result_loc);
    quad_handler_->emit_next_quad(IOPCode::ASSIGN, lvalue, nullptr, temp, result_loc);
    return temp;
}

inline bool
AssignBuilder::try_record_const_value(const Expr *const lvalue, const Expr *const rvalue)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);

    if (!SemUtils::is_const_expr(rvalue))
        return false;
    if (lvalue->type != Expr::Type::VARIABLE)
        return false;

    const VariableExpr *const var_expr = static_cast<const VariableExpr *>(lvalue);
    if (!var_expr->symbol->is_variable())
        return false;

    const Variable *const var_symbol = static_cast<const Variable *>(var_expr->symbol);
    SymbolTable::override_set_const_value(var_symbol, static_cast<const ConstExpr *>(rvalue));
    return true;
}

inline
BasicBuilder::BasicBuilder(Options &&options, const SemanticSystemServices &services)
    : options_(options),
      dr_(REQUIRE_PTR(services.dr)),
      expr_maker_(REQUIRE_PTR(services.expr_maker)),
      expr_folder_(REQUIRE_PTR(services.expr_folder)),
      snitch_(REQUIRE_PTR(services.expr_snitch)),
      quad_handler_(REQUIRE_PTR(services.quad_handler)),
      backpatcher_(REQUIRE_PTR(services.backpatcher)) {}

inline const Expr *
BasicBuilder::build_uminus(
    const Expr *expr,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    snitch_->report_if_not_arithmetic_expr(IOPCode::UMINUS, expr, OperandSide::UNARY);

    if (options_.propagate_constant_variable && expr->type == Expr::Type::VARIABLE)
    {
        const Variable *const var_symbol =
                static_cast<const Variable *>(
                    static_cast<const VariableExpr *>(expr)->symbol);
        DEBUG_SMART_ASSERT(!!var_symbol);
        if (var_symbol->has_const_value())
            expr = var_symbol->get_const_expr();
    }

    if (options_.fold_arithmetic && SemUtils::is_const_arithmetic_expr(expr))
        return expr_folder_->fold_uminus(expr, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next_quad(IOPCode::UMINUS, expr, nullptr, arithmetic_expr, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::build_arithmetic(
    const IOPCode iopc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)

{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    snitch_->report_if_not_arithmetic_expr(iopc, lhs, OperandSide::LEFT);
    snitch_->report_if_not_arithmetic_expr(iopc, rhs, OperandSide::RIGHT);

    if (options_.fold_arithmetic &&
        SemUtils::is_const_arithmetic_expr(lhs) &&
        SemUtils::is_const_arithmetic_expr(rhs))
        return expr_folder_->fold_arithmetic(iopc, lhs, rhs, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next_quad(iopc, lhs, rhs, arithmetic_expr, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::build_relational(
    const IOPCode iopc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    snitch_->report_if_not_relational(iopc, lhs, OperandSide::LEFT);
    snitch_->report_if_not_relational(iopc, rhs, OperandSide::RIGHT);

    if (options_.fold_relational &&
        SemUtils::is_relational_equality_iopcode(iopc) &&
        SemUtils::is_static_expr(lhs) &&
        SemUtils::is_static_expr(rhs))
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
BasicBuilder::build_logical_not(const Expr *const expr, const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr, SemUtils::is_in_bool_form(expr));
    BoolExpr *const bool_result_expr = expr_maker_->make_bool_expr(result_loc);
    bool_result_expr->true_list = static_cast<const BoolExpr *>(expr)->false_list;
    bool_result_expr->false_list = static_cast<const BoolExpr *>(expr)->true_list;
    return bool_result_expr;
}

inline const Expr *
BasicBuilder::build_logical_and(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(
        !!lhs, !!rhs,
        SemUtils::is_in_bool_form(lhs),
        SemUtils::is_in_bool_form(rhs)
    );

    if (options_.fold_logical)
        if (SemUtils::is_const_bool_expr(lhs) || SemUtils::is_const_bool_expr(rhs))
            return expr_folder_->fold_logical_and(lhs, rhs);
    return backpatcher_->resolve_lazy_bool_expr<Backpatcher::AndStrategy>(lhs, rhs, result_loc);
}

inline const Expr *
BasicBuilder::build_logical_or(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(
        !!lhs, !!rhs,
        SemUtils::is_in_bool_form(lhs),
        SemUtils::is_in_bool_form(rhs)
    );
    if (options_.fold_logical)
        if (SemUtils::is_const_bool_expr(lhs) || SemUtils::is_const_bool_expr(rhs))
            return expr_folder_->fold_logical_or(lhs, rhs);
    return backpatcher_->resolve_lazy_bool_expr<Backpatcher::OrStrategy>(lhs, rhs, result_loc);
}

inline
ConstBuilder::ConstBuilder(const SemanticSystemServices &services)
    : dr_(REQUIRE_PTR(services.dr)),
      expr_maker_(REQUIRE_PTR(services.expr_maker)) {}

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
} // namespace Alpha
#endif // EXPR_BUILDERS_HPP
