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
        static std::vector<LabelID> &backpatch_list(const BoolExpr *e) { return e->false_list; }
        static std::vector<LabelID> &merge_lhs_list(const BoolExpr *e) { return e->true_list; }
        static std::vector<LabelID> &merge_rhs_list(const BoolExpr *e) { return e->true_list; }
        static std::vector<LabelID> &assign_list(const BoolExpr *e) { return e->false_list; }
    };

    struct AndStrategy
    {
        static std::vector<LabelID> &backpatch_list(const BoolExpr *e) { return e->true_list; }
        static std::vector<LabelID> &merge_lhs_list(const BoolExpr *e) { return e->false_list; }
        static std::vector<LabelID> &merge_rhs_list(const BoolExpr *e) { return e->false_list; }
        static std::vector<LabelID> &assign_list(const BoolExpr *e) { return e->true_list; }
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

    [[nodiscard]] const Expr *build_pre_inc(const Expr *lvalue, SourceLocation result_loc);  // ++i
    [[nodiscard]] const Expr *build_post_inc(const Expr *lvalue, SourceLocation lvalue_loc); // i++
    [[nodiscard]] const Expr *build_pre_dec(const Expr *lvalue, SourceLocation lvalue_loc);  // --i
    [[nodiscard]] const Expr *build_post_dec(const Expr *lvalue, SourceLocation lvalue_loc); // i--

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
    [[nodiscard]] bool try_record_const_value(const Expr *lvalue, const Expr *rvalue);
};

class BasicBuilder
{
public:
    struct Options
    {
        bool fold_arithmetic;
        bool fold_relational;
        bool fold_logical;
        bool constant_propagation;
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


    template<typename... Exprs>
    [[nodiscard]] bool should_fold_arithmetic(const Exprs &... exprs);
    template<typename... Εxprs>
    [[nodiscard]] bool should_fold_relational_arithmetic(IOPCode iopc, const Εxprs &... exprs);
    template<typename... Εxprs>
    [[nodiscard]] bool should_fold_relational_equality(IOPCode iopc, const Εxprs &... exprs);
    template<typename... Exprs>
    [[nodiscard]] bool should_fold_logical(const Exprs &... exprs);
    [[nodiscard]] bool should_propagate_const();

    [[nodiscard]] static const Expr *try_propagate_const(const Expr *expr);
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
    const BoolExpr *const left_bool = static_cast<const BoolExpr *>(lhs);
    const BoolExpr *const right_bool = static_cast<const BoolExpr *>(rhs);
    const BoolExpr *const bool_result_expr = expr_maker_->make_bool_expr(result_loc);

    // Patching left side.
    DEBUG_SMART_ASSERT(!parse_cache_->short_circuit_jump_stack.empty());
    for (const LabelID quad_label: Strategy::backpatch_list(left_bool))
        quad_handler_->patch_quad(quad_label, parse_cache_->short_circuit_jump_stack.top());
    parse_cache_->short_circuit_jump_stack.pop();
    Strategy::backpatch_list(left_bool).clear();

    // Merging right side.
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

    DEBUG_SMART_ASSERT(!!bool_expr->var_symbol);

    qh->patch_list(bool_expr->true_list, qh->next_quad_label());
    qh->emit_next_quad(IOPCode::ASSIGN, &expr_maker_->static_true, nullptr, expr, expr->loc);
    qh->emit_next_quad(IOPCode::JUMP, nullptr, nullptr, nullptr, expr->loc, 2);
    quad_handler_->patch_list(bool_expr->false_list, quad_handler_->next_quad_label());
    qh->emit_next_quad(IOPCode::ASSIGN, &expr_maker_->static_false, nullptr, expr, expr->loc);
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

inline const Expr *
AssignBuilder::build_pre_inc(const Expr *const lvalue, const SourceLocation result_loc)
{
    if (!SemUtils::is_lvalue_expr(lvalue))
        dr_->report_operator_on_non_lvalue("increment", "++", result_loc);
    if (lvalue->type == Expr::Type::TABLE_ITEM)
    {
        const auto *const ti_lvalue = static_cast<const TableItemExpr *>(lvalue);
        const Expr *const result_var = ss_bridge_->emit_quad_if_table_item(ti_lvalue); // EMITS
        quad_handler_->emit_next_quad(
            IOPCode::ADD, result_var,& expr_maker_->static_int_1, result_var, result_loc);
        quad_handler_->emit_next_quad(
            IOPCode::TABLESETELEM, ti_lvalue, ti_lvalue->index, result_var, result_loc);
        return result_var;
    }
    else // This case runs even in error cases. (Do we want that? Well IR will definitely be wrong..., but it shouldn't crash)
        // Can you suggest a way I could return a singleton(s) known to be faulty (throw on error) Any compiler doing that?
    {
        quad_handler_->emit_next_quad(
            IOPCode::ADD, lvalue, &expr_maker_->static_int_1, lvalue, result_loc);
        const Expr *const result_var  = expr_maker_->make_arithmetic_expr(result_loc);
        quad_handler_->emit_next_quad(IOPCode::ASSIGN, lvalue, nullptr, result_var, result_loc);
        return result_var;
    }
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
    const Expr *temp_var = ss_bridge_->emit_quad_if_table_item(lvalue); // !CERTAIN EMIT!
    DEBUG_SMART_ASSERT(temp_var->type == Expr::Type::VARIABLE);
    const VarSymbol *temp_symbol = static_cast<const VariableExpr *>(temp_var)->var_symbol;
    return expr_maker_->make_assign_expr(result_loc, temp_symbol);
}

inline const Expr *
AssignBuilder::handle_direct_assignment(
    const Expr *const lvalue,
    const Expr *const rvalue,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);

    if (try_record_const_value(lvalue, rvalue))
        return lvalue;

    // TODO: check todo 52 (on how to make this only when needed)
    const Expr *const temp = expr_maker_->make_assign_expr(result_loc, parse_ctx_->new_temp());
    quad_handler_->emit_next_quad(IOPCode::ASSIGN, rvalue, nullptr, lvalue, result_loc);
    quad_handler_->emit_next_quad(IOPCode::ASSIGN, lvalue, nullptr, temp, result_loc);
    return temp;
}

inline bool
AssignBuilder::try_record_const_value(const Expr *const lvalue, const Expr *const rvalue)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);
    #ifndef ALL_OPTIMIZATIONS_ENABLED_BUILD
    if (!options_.record_constant_variables)
        return false;
    #endif
    if (!SemUtils::is_const_expr(rvalue))
        return false;
    if (lvalue->type != Expr::Type::VARIABLE)
        return false;

    const VariableExpr *const var_expr = static_cast<const VariableExpr *>(lvalue);
    SymbolTable::override_set_const_value(
        var_expr->var_symbol, static_cast<const ConstExpr *>(rvalue));
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

    if (should_propagate_const())
        expr = try_propagate_const(expr);
    if (should_fold_arithmetic(expr))
        return expr_folder_->fold_uminus(expr, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next_quad(IOPCode::UMINUS, expr, nullptr, arithmetic_expr, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::build_arithmetic(
    const IOPCode iopc,
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)

{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    snitch_->report_if_not_arithmetic_expr(iopc, lhs, OperandSide::LEFT);
    snitch_->report_if_not_arithmetic_expr(iopc, rhs, OperandSide::RIGHT);

    if (should_propagate_const())
    {
        lhs = try_propagate_const(lhs);
        rhs = try_propagate_const(rhs);
    }
    if (should_fold_arithmetic(lhs, rhs))
        return expr_folder_->fold_arithmetic(iopc, lhs, rhs, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next_quad(iopc, lhs, rhs, arithmetic_expr, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::build_relational(
    const IOPCode iopc,
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    snitch_->report_if_not_relational(iopc, lhs, OperandSide::LEFT);
    snitch_->report_if_not_relational(iopc, rhs, OperandSide::RIGHT);

    if (should_propagate_const())
    {
        lhs = try_propagate_const(lhs);
        rhs = try_propagate_const(rhs);
    }
    if (should_fold_relational_arithmetic(iopc, lhs, rhs))
        return expr_folder_->fold_relational_arithmetic(iopc, lhs, rhs, result_loc);
    if (should_fold_relational_equality(iopc, lhs, rhs))
        return expr_folder_->fold_relational_equality(iopc, lhs, rhs, result_loc);

    const BoolExpr *result_expr = expr_maker_->make_bool_expr(result_loc);
    result_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(iopc, lhs, rhs, nullptr, result_loc);
    result_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(IOPCode::JUMP, nullptr, nullptr, nullptr, result_loc);
    return result_expr;
}

inline const Expr *
BasicBuilder::build_logical_not(const Expr *expr, const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr, SemUtils::is_in_bool_form(expr));

    if (should_propagate_const())
        expr = try_propagate_const(expr);
    if (SemUtils::is_const_bool_expr(expr))
        return expr_folder_->fold_logical_not(expr, result_loc);

    const BoolExpr *const bool_result_expr = expr_maker_->make_bool_expr(result_loc);
    bool_result_expr->true_list = static_cast<const BoolExpr *>(expr)->false_list;
    bool_result_expr->false_list = static_cast<const BoolExpr *>(expr)->true_list;
    return bool_result_expr;
}

inline const Expr *
BasicBuilder::build_logical_and(
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(
        !!lhs, !!rhs,
        SemUtils::is_in_bool_form(lhs),
        SemUtils::is_in_bool_form(rhs)
    );

    if (should_propagate_const())
    {
        lhs = try_propagate_const(lhs);
        rhs = try_propagate_const(rhs);
    }
    if (should_fold_logical(lhs, rhs))
        return expr_folder_->fold_logical_and(lhs, rhs, result_loc);

    return backpatcher_->resolve_lazy_bool_expr<Backpatcher::AndStrategy>(lhs, rhs, result_loc);
}

inline const Expr *
BasicBuilder::build_logical_or(
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(
        !!lhs, !!rhs,
        SemUtils::is_in_bool_form(lhs),
        SemUtils::is_in_bool_form(rhs)
    );

    if (should_propagate_const())
    {
        lhs = try_propagate_const(lhs);
        rhs = try_propagate_const(rhs);
    }
    if (should_fold_logical(lhs, rhs))
        return expr_folder_->fold_logical_or(lhs, rhs, result_loc);

    return backpatcher_->resolve_lazy_bool_expr<Backpatcher::OrStrategy>(lhs, rhs, result_loc);
}

inline const Expr *
BasicBuilder::try_propagate_const(const Expr *const expr)
{
    if (expr->type != Expr::Type::VARIABLE)
        return expr;
    const VarSymbol *const var_symbol = static_cast<const VariableExpr *>(expr)->var_symbol;
    DEBUG_SMART_ASSERT(!!var_symbol); // All VariableExpr must be tied to a Variable(Symbol);
    if (!var_symbol->has_const_value())
        return expr;
    return var_symbol->get_const_expr();
}

template<typename... Exprs>
bool BasicBuilder::should_fold_arithmetic(const Exprs &... exprs)
{
    static_assert(sizeof...(Exprs) >= 1, "should_fold_arithmetic: expects at least 1 const Expr *");
    static_assert(sizeof...(Exprs) <= 2, "should_fold_arithmetic: expects at max 2 const Expr *");
    static_assert((std::is_same_v<Exprs, const Expr *> && ...),
                  "should_fold_arithmetic: expects all arguments to be const Expr *");

    // We fold the variadic exprs into a single `and` joined expression.
    return options_.fold_arithmetic && (SemUtils::is_const_arithmetic_expr(exprs) && ...);
}

template<typename... Exprs>
bool BasicBuilder::should_fold_relational_arithmetic(const IOPCode iopc, const Exprs &... exprs)
{
    static_assert(sizeof...(Exprs) == 2,
                  "should_fold_relational_arithmetic: expects exactly 2 const Expr *");
    static_assert((std::is_same_v<Exprs, const Expr *> && ...),
                  "should_fold_relational_arithmetic: expects all arguments to be const Expr *");

    // We fold the variadic exprs into a single `and` joined expression.
    return options_.fold_relational &&
           SemUtils::is_relational_arithmetic_iopcode(iopc) &&
           (SemUtils::is_const_arithmetic_expr(exprs) && ...);
}

template<typename... Exprs>
bool BasicBuilder::should_fold_relational_equality(const IOPCode iopc, const Exprs &... exprs)
{
    static_assert(sizeof...(Exprs) == 2,
                  "should_fold_relational_equality: expects exactly 2 const Expr *");
    static_assert((std::is_same_v<Exprs, const Expr *> && ...),
                  "should_fold_relational_equality: expects all arguments to be const Expr *");

    // We fold the variadic exprs into a single `and` joined expression.
    return options_.fold_relational &&
           SemUtils::is_relational_equality_iopcode(iopc) &&
           (SemUtils::is_static_expr(exprs) && ...);
}

template<typename... Exprs>
bool BasicBuilder::should_fold_logical(const Exprs &... exprs)
{
    static_assert(sizeof...(Exprs) >= 1, "should_fold_logical: expects at least 1 const Expr *");
    static_assert(sizeof...(Exprs) <= 2, "should_fold_logical: expects at max 2 const Expr *");
    static_assert((std::is_same_v<Exprs, const Expr *> && ...),
                  "should_fold_logical: expects all arguments to be const Expr *");

    // We fold the variadic exprs into a single `or` joined expression.
    // Why `or` because in logical operators AND, OR, we can even do partial folding.
    // e.g.: true and var => var
    return options_.fold_logical && (SemUtils::is_const_bool_expr(exprs) || ...);
}

inline bool BasicBuilder::should_propagate_const()
{
    #ifndef ALL_OPTIMIZATIONS_ENABLED_BUILD
    return true;
    #endif
    return options_.constant_propagation;
}


inline
ConstBuilder::ConstBuilder(const SemanticSystemServices &services)
    : dr_(REQUIRE_PTR(services.dr)),
      expr_maker_(REQUIRE_PTR(services.expr_maker)) {}

inline const Expr *
ConstBuilder::build_true_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(loc, true);
}

inline const Expr *
ConstBuilder::build_false_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(loc, false);
}

inline const Expr *
ConstBuilder::build_int_expr(const AlphaInt value, const SourceLocation loc)
{
    return expr_maker_->make_const_int_expr(loc, value);
}

inline const Expr *
ConstBuilder::build_float_expr(const AlphaFloat value, const SourceLocation loc)
{
    return expr_maker_->make_const_float_expr(loc, value);
}

inline const Expr *
ConstBuilder::build_string_expr(const char *const value, const SourceLocation loc)
{
    return expr_maker_->make_const_string_expr(loc, value);
}

inline const Expr *
ConstBuilder::build_nil_expr(const SourceLocation loc)
{
    return expr_maker_->make_nil_expr(loc);
}
} // namespace Alpha
#endif // EXPR_BUILDERS_HPP
