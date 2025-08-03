#ifndef EXPR_BUILDERS_HPP
#define EXPR_BUILDERS_HPP
#include <functional>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include "L1_driver/semantic_system_support.hpp"
#include "L3_ir_infra/expr_folder.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/quad_handler.hpp"
#include "parser/ir.hpp"
#include "parser/semantic_utils.hpp"
#include "semantic_subsystem.hpp"

namespace Alpha
{
class AggregateBuilder
{
    friend class SemanticSystem;

private:
    class Restricted final : private SemanticSubsystem
    {
        friend class AggregateBuilder;

    private:
        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        // List related (candidate for submodule)
        [[nodiscard]] static ExprList *build_expr_list();
        [[nodiscard]] static ExprList *build_expr_list(const Expr *head_expr);
        [[nodiscard]] static ExprList *extend_expr_list(ExprList *elist, const Expr *next_expr);
        static void delete_expr_list(ExprList *&elist);

        // Dict related (candidate for submodule)
        [[nodiscard]] static const ExprPair *build_expr_pair(const Expr *first, const Expr *second);
        [[nodiscard]] static DictList *build_dict_list();
        [[nodiscard]] static DictList *build_dict_list(const ExprPair *head_pair);
        [[nodiscard]] static DictList *extend_dict_list(DictList *dlist, const ExprPair *next_pair);
        void static delete_dict_list(DictList *&dlist);

        [[nodiscard]] const Expr *build_table_list_consuming(
            ExprList *elist, SourceLocation table_list_loc);
        [[nodiscard]] const Expr *build_table_dict_consuming(
            DictList *&dlist, SourceLocation table_dict_loc);
    };

    Restricted DISPATCH_TARGET;

    explicit AggregateBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(build_expr_list);
    DISPATCH_SLAVE_METHOD_CALL(build_expr_pair);
    DISPATCH_SLAVE_METHOD_CALL(build_dict_list);
    DISPATCH_SLAVE_METHOD_CALL(extend_expr_list);
    DISPATCH_SLAVE_METHOD_CALL(extend_dict_list);
    DISPATCH_SLAVE_METHOD_CALL(build_table_list_consuming);
    DISPATCH_SLAVE_METHOD_CALL(build_table_dict_consuming);
    DISPATCH_DEFINE_HANDLER_END();
};

class AssignBuilder
{
    friend class SemanticSystem;

private:
    struct Options
    {
        bool record_constant_variables;
    };

    class Restricted final : private SemanticSubsystem
    {
        friend class AssignBuilder;

    private:
        enum class OpVariant { PRE, POST };

        struct IncPolicy
        {
            static constexpr auto iopc = IOPCode::ADD;
            static constexpr char op_name[] = "increment";
            static constexpr char op_symbol[] = "++";
        };

        struct DecPolicy
        {
            static constexpr auto iopc = IOPCode::SUB;
            static constexpr char op_name[] = "decrement";
            static constexpr char op_symbol[] = "--";
        };

        const Options options_;

        Restricted(Options &&options, const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        [[nodiscard]] const Expr *build_assignment(
            const Expr *lvalue, const Expr *rvalue, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_pre_inc(const Expr *lvalue, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_post_inc(const Expr *lvalue, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_pre_dec(const Expr *lvalue, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_post_dec(const Expr *lvalue, SourceLocation result_loc);

        [[nodiscard]] bool validate_lvalue_assignment(
            const Expr *lvalue, SourceLocation assign_loc);
        [[nodiscard]] bool try_record_const_expr(const Expr *lvalue, const Expr *rvalue);
        [[nodiscard]] const Expr *handle_table_item_assignment(
            const Expr *lvalue, const Expr *rvalue, SourceLocation result_loc);
        [[nodiscard]] const Expr *handle_direct_assignment(
            const Expr *lvalue, const Expr *rvalue, SourceLocation result_loc);

        template<OpVariant op_variant, typename Policy>
        [[nodiscard]] const Expr *build_inc_dec(const Expr *lvalue, SourceLocation result_loc);
        template<typename Policy>
        [[nodiscard]] const Expr *handle_pre_inc_dec(
            const Expr *lvalue, SourceLocation result_loc);
        template<typename Policy>
        [[nodiscard]] const Expr *handle_post_inc_dec(
            const Expr *lvalue, SourceLocation result_loc);
    };

    Restricted DISPATCH_TARGET;

    AssignBuilder(Options &&options, const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(build_assignment);
    DISPATCH_SLAVE_METHOD_CALL(build_pre_inc);
    DISPATCH_SLAVE_METHOD_CALL(build_post_inc);
    DISPATCH_SLAVE_METHOD_CALL(build_pre_dec);
    DISPATCH_SLAVE_METHOD_CALL(build_post_dec);
    DISPATCH_DEFINE_HANDLER_END();
};

class BasicBuilder
{
    friend class SemanticSystem;

private:
    struct Options
    {
        bool fold_arithmetic;
        bool fold_relational;
        bool fold_logical;
        bool constant_propagation;
    };

    class Restricted final : private SemanticSubsystem
    {
        friend class BasicBuilder;

    private:
        struct OrShortCircuitPolicy
        {
            static auto &backpatch_list(const BoolExpr *e) { return e->false_list; }
            static auto &merge_lhs_list(const BoolExpr *e) { return e->true_list; }
            static auto &merge_rhs_list(const BoolExpr *e) { return e->true_list; }
            static auto &assign_list(const BoolExpr *e) { return e->false_list; }
        };

        struct AndShortCircuitPolicy
        {
            static auto &backpatch_list(const BoolExpr *e) { return e->true_list; }
            static auto &merge_lhs_list(const BoolExpr *e) { return e->false_list; }
            static auto &merge_rhs_list(const BoolExpr *e) { return e->false_list; }
            static auto &assign_list(const BoolExpr *e) { return e->true_list; }
        };

        const Options options_;
        std::stack<LabelID> short_circuit_jump_stack_;

        Restricted(Options &&options, const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void mark_short_circuit_jump_point();
        [[nodiscard]] const Expr *build_uminus(const Expr *expr, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_arithmetic(
            IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_relational(
            IOPCode iopc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_logical_not(const Expr *expr, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_logical_and(
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_logical_or(
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

        template<typename BackpatchingPolicy>
        [[nodiscard]] const Expr *build_short_circuit_bool_expr(
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

        [[nodiscard]] bool should_propagate_const();

        [[nodiscard]] static const Expr *try_propagate_const(const Expr *expr);

        [[nodiscard]] bool validate_arithmetic_expr(
            IOPCode iopc, const Expr *expr, OperandSide op_side);
        [[nodiscard]] bool validate_relational_expr(
            IOPCode iopc, const Expr *expr, OperandSide op_side);
        [[nodiscard]] bool validate_possible_division(
            IOPCode iopc, const Expr *rhs, SourceLocation division_loc);

        void warn_if_lossy_conversion_int_to_float(AlphaInt value, SourceLocation conversion_loc);
    };

    Restricted DISPATCH_TARGET;

    BasicBuilder(Options &&options, const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(build_uminus);
    DISPATCH_SLAVE_METHOD_CALL(build_arithmetic);
    DISPATCH_SLAVE_METHOD_CALL(build_relational);
    DISPATCH_SLAVE_METHOD_CALL(build_relational);
    DISPATCH_SLAVE_METHOD_CALL(build_logical_not);
    DISPATCH_SLAVE_METHOD_CALL(build_logical_and);
    DISPATCH_SLAVE_METHOD_CALL(build_logical_or);
    DISPATCH_DEFINE_HANDLER_END();
};

class ConstBuilder
{
    friend class SemanticSystem;

private:
    class Restricted final : private SemanticSubsystem
    {
        friend class ConstBuilder;

    private:
        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        [[nodiscard]] const Expr *build_true_expr(SourceLocation loc);
        [[nodiscard]] const Expr *build_false_expr(SourceLocation loc);
        [[nodiscard]] const Expr *build_int_expr(AlphaInt value, SourceLocation loc);
        [[nodiscard]] const Expr *build_float_expr(AlphaFloat value, SourceLocation loc);
        [[nodiscard]] const Expr *build_string_expr(const char *value, SourceLocation loc);
        [[nodiscard]] const Expr *build_nil_expr(SourceLocation loc);
    };

    Restricted DISPATCH_TARGET;

    explicit ConstBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(build_true_expr);
    DISPATCH_SLAVE_METHOD_CALL(build_false_expr);
    DISPATCH_SLAVE_METHOD_CALL(build_false_expr);
    DISPATCH_SLAVE_METHOD_CALL(build_false_expr);
    DISPATCH_SLAVE_METHOD_CALL(build_int_expr);
    DISPATCH_SLAVE_METHOD_CALL(build_float_expr);
    DISPATCH_SLAVE_METHOD_CALL(build_string_expr);
    DISPATCH_SLAVE_METHOD_CALL(build_nil_expr);
    DISPATCH_DEFINE_HANDLER_END();
};

class FunctionBuilder
{
    friend class SemanticSystem;

private:
    class Restricted final : private SemanticSubsystem
    {
        friend class FunctionBuilder;

    private:
        struct
        {
            std::string id;
            SourceLocation location;
        } function_draft;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void begin_anonymous(SourceLocation anonymous_loc, const char *id_name = "");
    };

    Restricted DISPATCH_TARGET;

    explicit FunctionBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(begin_anonymous);
    DISPATCH_DEFINE_HANDLER_END();
};

inline ExprList *
AggregateBuilder::Restricted::build_expr_list() { return new ExprList(); }

inline ExprList *
AggregateBuilder::Restricted::build_expr_list(const Expr *const head_expr)
{
    DEBUG_SMART_ASSERT(!!head_expr);
    return extend_expr_list(build_expr_list(), head_expr);
}

inline ExprList *
AggregateBuilder::Restricted::extend_expr_list(
    ExprList *const elist,
    const Expr *const next_expr)
{
    DEBUG_SMART_ASSERT(!!elist, !!next_expr);
    elist->push_back(next_expr);
    return elist;
}

// Passed by reference to nullify after deletion -- avoids leaving a dangling pointer.
inline void
AggregateBuilder::Restricted::delete_expr_list(ExprList *&elist)
{
    // Note: Do NOT delete the expressions in ExprList -- those are handler by ExprMaker.
    delete elist;
    DEBUG_NULLIFY(elist);
}

inline const ExprPair *
AggregateBuilder::Restricted::build_expr_pair(const Expr *const first, const Expr *const second)
{
    DEBUG_SMART_ASSERT(!!first, !!second);
    // TODO: can you make this `new const` ? Can you delete ptr afterwards?
    // Without const_cast() checks when at end of project
    return new ExprPair(first, second);
}

inline DictList *
AggregateBuilder::Restricted::build_dict_list() { return new DictList(); }

inline DictList *
AggregateBuilder::Restricted::build_dict_list(const ExprPair *const head_pair)
{
    DEBUG_SMART_ASSERT(!!head_pair);
    return extend_dict_list(build_dict_list(), head_pair);
}

inline DictList *
AggregateBuilder::Restricted::extend_dict_list(
    DictList *const dlist,
    const ExprPair *const next_pair)
{
    DEBUG_SMART_ASSERT(!!dlist, !!next_pair);
    dlist->push_back(next_pair);
    return dlist;
}

// Passed by reference to nullify after deletion -- avoids leaving a dangling pointer.
inline void
AggregateBuilder::Restricted::delete_dict_list(DictList *&dlist)
{
    // Note: Do NOT delete the expressions in ExprPair -- those are handler by ExprMaker.
    for (const ExprPair *pair: *dlist)
        delete pair; // Shallow delete, it does NOT delete the expressions it's holding.
    delete dlist;
    DEBUG_NULLIFY(dlist);
}

inline const Expr *
AggregateBuilder::Restricted::build_table_list_consuming(
    ExprList *elist,
    const SourceLocation table_list_loc)
{
    DEBUG_SMART_ASSERT(!!elist);
    auto *const qh = quad_handler_; // Short alias to improve readability and reduce verbosity

    const NewTableExpr *const new_table_expr = expr_maker_->make_new_table_expr(table_list_loc);
    qh->emit_next(IOPCode::TABLECREATE, new_table_expr, nullptr, nullptr, table_list_loc);

    // Emit list's items.
    u32 list_index = 0;
    for (auto expr_it = elist->crbegin(); expr_it != elist->crend(); ++expr_it)
    {
        const Expr *index_expr = expr_maker_->make_const_int_expr((*expr_it)->loc, list_index++);
        qh->emit_next(IOPCode::TABLESETELEM, new_table_expr, index_expr, *expr_it, (*expr_it)->loc);
    }

    // Delete elist after use — it must not be used again
    delete_expr_list(elist);

    return new_table_expr;
}

inline const Expr *
AggregateBuilder::Restricted::build_table_dict_consuming(
    DictList *&dlist,
    const SourceLocation table_dict_loc)
{
    DEBUG_SMART_ASSERT(!!dlist);
    auto *const qh = quad_handler_; // Short alias to improve readability and reduce verbosity

    const Expr *const new_table_expr = expr_maker_->make_new_table_expr(table_dict_loc);
    qh->emit_next(IOPCode::TABLECREATE, nullptr, nullptr, new_table_expr, table_dict_loc);

    // Emit dict's items.
    for (auto it = dlist->crbegin(); it != dlist->crend(); ++it)
        qh->emit_next(IOPCode::TABLESETELEM, (*it)->first, (*it)->second, new_table_expr, k_no_loc);

    // Delete elist after use — it must not be used again
    delete_dict_list(dlist);

    return new_table_expr;
}

inline const Expr *
AssignBuilder::Restricted::build_assignment(
    const Expr *const lvalue,
    const Expr *const rvalue,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);
    if (!validate_lvalue_assignment(lvalue, result_loc))
        return nullptr;

    return lvalue->type == Expr::Type::TABLE_ITEM
           ? handle_table_item_assignment(lvalue, rvalue, result_loc)
           : handle_direct_assignment(lvalue, rvalue, result_loc);
}

inline const Expr *
AssignBuilder::Restricted::build_pre_inc(const Expr *const lvalue, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::PRE, IncPolicy>(lvalue, result_loc);
}

inline const Expr *
AssignBuilder::Restricted::build_post_inc(const Expr *const lvalue, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::POST, IncPolicy>(lvalue, result_loc);
}

inline const Expr *
AssignBuilder::Restricted::build_pre_dec(const Expr *const lvalue, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::PRE, DecPolicy>(lvalue, result_loc);
}

inline const Expr *
AssignBuilder::Restricted::build_post_dec(const Expr *const lvalue, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::POST, DecPolicy>(lvalue, result_loc);
}

inline bool
AssignBuilder::Restricted::validate_lvalue_assignment(
    const Expr *const lvalue,
    const SourceLocation assign_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue);
    if (!SemUtils::is_lvalue_expr(lvalue))
    {
        dr_->report_assign_lhs_not_lvalue(lvalue->type, lvalue->loc);
        return false;
    }
    DEBUG_SMART_ASSERT(lvalue->has_symbol); // If here. Its Lvalue and all lvalues have symbols.
    const Symbol *const lv_symbol = static_cast<const ExprWSymbol *>(lvalue)->symbol;
    if (lv_symbol->type == Symbol::Type::LIBRARY_FUNCTION)
    {
        dr_->report_assign_to_libfunc(lv_symbol->name, assign_loc);
        return false;
    }
    if (lv_symbol->type == Symbol::Type::PROGRAM_FUNCTION)
    {
        dr_->report_assign_to_func(lv_symbol->name, assign_loc, lv_symbol->loc);
        return false;
    }
    return true;
}

// TODO: do we propagate assignment of assignment like x = y = z = 5? If NOT
// We might need to let Expr::Type::ASSIGN_EXPR
inline bool
AssignBuilder::Restricted::try_record_const_expr(const Expr *const lvalue, const Expr *const rvalue)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);
    #ifndef ALL_OPTIMIZATIONS_ENABLED_BUILD
    if (!options_.record_constant_variables)
        return false;
    #endif
    if (lvalue->type != Expr::Type::VARIABLE)
        return false;

    const auto *const var_symbol = static_cast<const VariableExpr *>(lvalue)->var_symbol;
    if (!SemUtils::is_const_expr(rvalue))
    {
        SymbolTable::clear_const_expr(var_symbol);
        return false;
    }
    SymbolTable::attach_const_expr(var_symbol, static_cast<const ConstExpr *>(rvalue));
    return true;
}

inline const Expr *
AssignBuilder::Restricted::handle_table_item_assignment(
    const Expr *const lvalue,
    const Expr *const rvalue,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);
    DEBUG_SMART_ASSERT(lvalue->type == Expr::Type::TABLE_ITEM);

    const auto *const ti = static_cast<const TableItemExpr *>(lvalue);
    quad_handler_->emit_next(IOPCode::TABLESETELEM, rvalue, ti, ti->index, result_loc);
    const Expr *temp_var = ss_bridge_->emit_tablegetelem_if_table_item(lvalue); // !CERTAIN EMIT!
    DEBUG_SMART_ASSERT(temp_var->type == Expr::Type::VARIABLE);
    const VarSymbol *temp_symbol = static_cast<const VariableExpr *>(temp_var)->var_symbol;
    return expr_maker_->make_assign_expr(result_loc, temp_symbol);
}

inline const Expr *
AssignBuilder::Restricted::handle_direct_assignment(
    const Expr *const lvalue,
    const Expr *const rvalue,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);

    if (try_record_const_expr(lvalue, rvalue))
        return lvalue; // Now lvalue's symbol carries rvalue.

    // TODO: check todo 52 (on how to make this only when needed)
    const Expr *const temp = expr_maker_->make_assign_expr(result_loc, parse_ctx_->new_temp());
    quad_handler_->emit_next(IOPCode::ASSIGN, lvalue, rvalue, nullptr, result_loc);
    quad_handler_->emit_next(IOPCode::ASSIGN, temp, lvalue, nullptr, result_loc);
    return temp;
}

template<AssignBuilder::Restricted::OpVariant op_variant, typename Policy>
const Expr *
AssignBuilder::Restricted::build_inc_dec(const Expr *const lvalue, const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> ||
                  std::is_same_v<Policy, DecPolicy>, "Expected INC or DEC policy");
    if (!SemUtils::is_lvalue_expr(lvalue))
    {
        dr_->report_operator_requires_lvalue(Policy::op_name, Policy::op_symbol, result_loc);
        return nullptr;
    }

    if constexpr (op_variant == OpVariant::PRE)
        return handle_pre_inc_dec<Policy>(lvalue, result_loc);
    else if constexpr (op_variant == OpVariant::POST)
        return handle_post_inc_dec<Policy>(lvalue, result_loc);
    else
        static_assert(false, "build_inc_dec(): Unknown OpVariant");
}

template<typename Policy>
const Expr *
AssignBuilder::Restricted::handle_pre_inc_dec(const Expr *lvalue, const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> ||
                  std::is_same_v<Policy, DecPolicy>, "Expected INC or DEC policy");
    auto *const qh = quad_handler_; // Short alias to improve readability and reduce verbosity

    const Expr *result = nullptr;
    if (lvalue->type == Expr::Type::TABLE_ITEM)
    {
        const auto *const ti_lvalue = static_cast<const TableItemExpr *>(lvalue);
        result = ss_bridge_->emit_tablegetelem_if_table_item(ti_lvalue); // EMITS!
        qh->emit_next(Policy::iopc, result, result, &k_static_int_1_expr, result_loc);
        qh->emit_next(IOPCode::TABLESETELEM, result, ti_lvalue, ti_lvalue->index, result_loc);
    }
    else
    {
        // TODO: HOOK: After you implemented logic to make assignment aware of if its happening,
        // inside a function parameter list (TODO 52), create this new arithmetic_expr (new temp)
        // only if inside assignment. NOTE! ONLY ENABLE THIS OPTIMIZATION IFF optimization options is passed.
        // DO NOT make it standard behavior.. you may get fucked in examination :D
        result = expr_maker_->make_arithmetic_expr(result_loc);
        qh->emit_next(Policy::iopc, lvalue, lvalue, &k_static_int_1_expr, result_loc);
        qh->emit_next(IOPCode::ASSIGN, result, lvalue, nullptr, result_loc);
    }
    return DEBUG_REQUIRE_PTR(result); // Check because we initialized with nullptr.
}

template<typename Policy>
const Expr *
AssignBuilder::Restricted::handle_post_inc_dec(const Expr *lvalue, const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> ||
                  std::is_same_v<Policy, DecPolicy>, "Expected INC or DEC policy");
    auto *const qh = quad_handler_; // Short alias to improve readability and reduce verbosity

    const Expr *result = expr_maker_->make_variable_expr(result_loc, parse_ctx_->new_temp());
    if (lvalue->type == Expr::Type::TABLE_ITEM)
    {
        const auto *const ti_lvalue = static_cast<const TableItemExpr *>(lvalue);
        const Expr *ti = ss_bridge_->emit_tablegetelem_if_table_item(lvalue); // EMITS!
        qh->emit_next(IOPCode::ASSIGN, result, ti, nullptr, result_loc);
        qh->emit_next(Policy::iopc, ti, ti, &k_static_int_1_expr, result_loc);
        qh->emit_next(IOPCode::TABLESETELEM, ti, ti_lvalue, ti_lvalue->index, result_loc);
    }
    else
    {
        qh->emit_next(IOPCode::ASSIGN, result, lvalue, nullptr, result_loc);
        qh->emit_next(Policy::iopc, lvalue, lvalue, &k_static_int_1_expr, result_loc);
    }
    return result;
}

inline void
BasicBuilder::Restricted::mark_short_circuit_jump_point()
{
    short_circuit_jump_stack_.push(quad_handler_->next_quad_label());
}

inline const Expr *
BasicBuilder::Restricted::build_uminus(
    const Expr *expr,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    expr_snitch_->validate_arithmetic_expr(IOPCode::UMINUS, expr, OperandSide::UNARY);

    if (should_propagate_const())
        expr = try_propagate_const(expr);
    if (should_fold_arithmetic(expr))
        return expr_folder_->fold_uminus(expr, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next(IOPCode::UMINUS, arithmetic_expr, expr, nullptr, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::Restricted::build_arithmetic(
    const IOPCode iopc,
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (!validate_arithmetic_expr(iopc, lhs, OperandSide::LEFT)) return nullptr;
    if (!validate_arithmetic_expr(iopc, rhs, OperandSide::RIGHT)) return nullptr;
    if (!validate_possible_division(iopc, rhs, result_loc)) return nullptr;

    if (should_propagate_const())
    {
        lhs = try_propagate_const(lhs);
        rhs = try_propagate_const(rhs);
    }
    if (should_fold_arithmetic(lhs, rhs))
        return expr_folder_->fold_arithmetic(iopc, lhs, rhs, result_loc);

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next(iopc, arithmetic_expr, lhs, rhs, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::Restricted::build_relational(
    const IOPCode iopc,
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (!validate_relational_expr(iopc, lhs, OperandSide::LEFT)) return nullptr;
    if (!validate_relational_expr(iopc, rhs, OperandSide::RIGHT)) return nullptr;

    if (should_propagate_const())
    {
        lhs = try_propagate_const(lhs);
        rhs = try_propagate_const(rhs);
    }
    if (should_fold_relational_arithmetic(iopc, lhs, rhs))
        return expr_folder_->fold_relational_arithmetic(iopc, lhs, rhs, result_loc);
    if (should_fold_relational_equality(iopc, lhs, rhs))
        return expr_folder_->fold_relational_equality(iopc, lhs, rhs, result_loc);
    if (const Expr *simplified = expr_folder_->try_simplify_relational_equality(
        iopc, lhs, rhs, result_loc))
        return simplified;

    const BoolExpr *result_expr = expr_maker_->make_bool_expr(result_loc);
    result_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(iopc, nullptr, lhs, rhs, result_loc);
    result_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, result_loc);
    return result_expr;
}

inline const Expr *
BasicBuilder::Restricted::build_logical_not(const Expr *expr, const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr, SemUtils::is_in_bool_form(expr));

    if (should_propagate_const())
        expr = try_propagate_const(expr);
    if (should_fold_logical(expr))
        return expr_folder_->fold_logical_not(expr, result_loc);

    const BoolExpr *const bool_result_expr = expr_maker_->make_bool_expr(result_loc);
    bool_result_expr->true_list = static_cast<const BoolExpr *>(expr)->false_list;
    bool_result_expr->false_list = static_cast<const BoolExpr *>(expr)->true_list;
    return bool_result_expr;
}

inline const Expr *
BasicBuilder::Restricted::build_logical_and(
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

    return build_short_circuit_bool_expr<AndShortCircuitPolicy>(lhs, rhs, result_loc);
}

inline const Expr *
BasicBuilder::Restricted::build_logical_or(
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

    return build_short_circuit_bool_expr<OrShortCircuitPolicy>(lhs, rhs, result_loc);
}

inline const Expr *
BasicBuilder::Restricted::try_propagate_const(const Expr *const expr)
{
    if (expr->type != Expr::Type::VARIABLE)
        return expr;
    const VarSymbol *const var_symbol = static_cast<const VariableExpr *>(expr)->var_symbol;
    DEBUG_SMART_ASSERT(!!var_symbol); // All VariableExpr must be tied to a Variable(Symbol);
    if (!var_symbol->has_const_value())
        return expr;
    return var_symbol->get_const_expr();
}

template<typename Policy>
const Expr *
BasicBuilder::Restricted::build_short_circuit_bool_expr(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, OrShortCircuitPolicy> ||
                  std::is_same_v<Policy, AndShortCircuitPolicy>, "Unknown backpatching policy");

    DEBUG_SMART_ASSERT(lhs->type == Expr::Type::BOOL_EXPR && rhs->type == Expr::Type::BOOL_EXPR);
    const BoolExpr *const lhs_bool = static_cast<const BoolExpr *>(lhs);
    const BoolExpr *const rhs_bool = static_cast<const BoolExpr *>(rhs);
    const BoolExpr *const bool_result_expr = expr_maker_->make_bool_expr(result_loc);

    // Patching left side.
    DEBUG_SMART_ASSERT(!short_circuit_jump_stack_.empty());
    for (const LabelID quad_label: Policy::backpatch_list(lhs_bool))
        quad_handler_->patch_quad(quad_label, short_circuit_jump_stack_.top());
    short_circuit_jump_stack_.pop();
    Policy::backpatch_list(lhs_bool).clear();

    // Merging right side.
    auto &lhs_merge = Policy::merge_lhs_list(lhs_bool);
    auto &rhs_merge = Policy::merge_rhs_list(rhs_bool);
    auto &result_merge = Policy::merge_lhs_list(bool_result_expr);
    // We could use merge_rhs too
    result_merge.reserve(lhs_merge.size() + rhs_merge.size());
    result_merge.insert(result_merge.end(), lhs_merge.begin(), lhs_merge.end());
    result_merge.insert(result_merge.end(), rhs_merge.begin(), rhs_merge.end());

    Policy::assign_list(bool_result_expr) = Policy::assign_list(rhs_bool);
    return bool_result_expr;
}

inline bool
BasicBuilder::Restricted::should_propagate_const()
{
    #ifndef ALL_OPTIMIZATIONS_ENABLED_BUILD
    return true;
    #endif
    return options_.constant_propagation;
}

inline bool
BasicBuilder::Restricted::validate_arithmetic_expr(
    const IOPCode iopc,
    const Expr *expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (SemUtils::is_arithmetic_convertible_expr(expr))
        return true;

    if (SemUtils::is_binary_arithmetic_iopcode(iopc))
        dr_->report_arith_op_nonarith_operand(
            op_side, SemUtils::arith_iopc_to_str_symbol(iopc), expr->type, expr->loc);
    else if (iopc == IOPCode::UMINUS)
        dr_->report_uminus_nonarith_operand(expr->type, expr->loc);
    else
        throw std::logic_error(ATTACH_CONTEXT("Expected arithmetic IOPCode (bin arith or uminus)"));
    return false;
}

inline bool
BasicBuilder::Restricted::validate_relational_expr(
    const IOPCode iopc,
    const Expr *const expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr,);
    // In Alpha everything is convertible to bool.
    // And operators == and != convert their operands to bool.
    if (SemUtils::is_relational_equality_iopcode(iopc))
        return true;
    // If here relational operator is:  < <= > >=
    if (SemUtils::is_arithmetic_convertible_expr(expr))
        return true;
    dr_->report_rel_op_nonarith_operand(
        op_side, SemUtils::rel_op_to_str(iopc), expr->type, expr->loc);
    return false;
}

inline bool
BasicBuilder::Restricted::validate_possible_division(
    const IOPCode iopc,
    const Expr *const rhs,
    const SourceLocation division_loc)
{
    if (iopc != IOPCode::DIV && iopc != IOPCode::MOD)
        return true;
    if (!SemUtils::is_const_0(rhs))
        return true;
    dr_->report_division_by_zero(division_loc);
    return false;
}

inline void
BasicBuilder::Restricted::warn_if_lossy_conversion_int_to_float(
    const AlphaInt value,
    const SourceLocation conversion_loc)
{
    if (!Utils::is_lossless_int_to_float<AlphaFloat>(value))
        dr_->report_implicit_int_to_float_loss(conversion_loc);
}

inline const Expr *
ConstBuilder::Restricted::build_true_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(loc, true);
}

inline const Expr *
ConstBuilder::Restricted::build_false_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(loc, false);
}

inline const Expr *
ConstBuilder::Restricted::build_int_expr(const AlphaInt value, const SourceLocation loc)
{
    return expr_maker_->make_const_int_expr(loc, value);
}

inline const Expr *
ConstBuilder::Restricted::build_float_expr(const AlphaFloat value, const SourceLocation loc)
{
    return expr_maker_->make_const_float_expr(loc, value);
}

inline const Expr *
ConstBuilder::Restricted::build_string_expr(const char *const value, const SourceLocation loc)
{
    return expr_maker_->make_const_string_expr(loc, value);
}

inline const Expr *
ConstBuilder::Restricted::build_nil_expr(const SourceLocation loc)
{
    return expr_maker_->make_nil_expr(loc);
}

inline void
FunctionBuilder::Restricted::begin_anonymous(
    const SourceLocation anonymous_loc,
    const char *const id_name)
{
    DEBUG_SMART_ASSERT(!!id_name);

    function_draft.id = id_name[0] != '\0' ? id_name : parse_ctx_->name_generator.new_anonymous();
    function_draft.location = anonymous_loc;
    parse_ctx_->space_handler.enter_space();
}
} // namespace Alpha
#endif // EXPR_BUILDERS_HPP
