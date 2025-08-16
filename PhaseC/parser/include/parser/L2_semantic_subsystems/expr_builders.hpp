#ifndef EXPR_BUILDERS_HPP
#define EXPR_BUILDERS_HPP

#include <functional>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include "L1_driver/semantic_system_support.hpp"
#include "L3_ir_infra/expr_optimizer.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/quad_handler.hpp"
#include "parser/semantic_utils.hpp"
#include "semantic_subsystem.hpp"
#include <parser/ir_opcode.gen.hpp>

namespace alpha
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
            ExprList *&elist, SourceLocation table_list_loc);
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
            static constexpr auto opc = ir::Opcode::ADD;
            static constexpr char op_name[] = "increment";
            static constexpr char op_symbol[] = "++";
        };

        struct DecPolicy
        {
            static constexpr auto opc = ir::Opcode::SUB;
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

        std::stack<LabelID> short_circuit_jump_stack_;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void mark_short_circuit_jump_point();
        [[nodiscard]] const Expr *build_uminus(const Expr *expr, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_arithmetic(
            ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_relational(
            ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_logical_not(const Expr *expr, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_logical_and(
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_logical_or(
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

        template<typename BackpatchingPolicy>
        [[nodiscard]] const Expr *build_short_circuit_bool_expr(
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

        [[nodiscard]] bool validate_arithmetic_expr(
            ir::Opcode opc, const Expr *expr, OperandSide op_side);
        [[nodiscard]] bool validate_relational_expr(
            ir::Opcode opc, const Expr *expr, OperandSide op_side);
        [[nodiscard]] bool validate_possible_division(
            ir::Opcode iropcode, const Expr *rhs, SourceLocation division_loc);

        // When I built the compile-time call dispatcher for Bison, I didn’t add support for template args.
        // Later, I made the optimizer fully templated. Rather than making it runtime-based,
        // I use a clean runtime to compile-time dispatcher for expr_optimizer's try_optimize()
        // Only arithmetic and relational builders take ir::Opcode as a runtime arg,
        // since they share logic with the opcode being the only varying part.
        [[nodiscard]] const Expr *try_optimize_arithmetic_expr(
            ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *try_optimize_relational_expr(
            ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

        void warn_if_lossy_conversion_int_to_float(AlphaInt value, SourceLocation conversion_loc);
    };

    Restricted DISPATCH_TARGET;

    explicit BasicBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(mark_short_circuit_jump_point);
    DISPATCH_SLAVE_METHOD_CALL(build_uminus);
    DISPATCH_SLAVE_METHOD_CALL(build_arithmetic);
    DISPATCH_SLAVE_METHOD_CALL(build_relational);
    DISPATCH_SLAVE_METHOD_CALL(build_logical_not);
    DISPATCH_SLAVE_METHOD_CALL(build_logical_and);
    DISPATCH_SLAVE_METHOD_CALL(build_logical_or);
    DISPATCH_DEFINE_HANDLER_END();
};

class CallBuilder
{
    friend class SemanticSystem;

private:
    class Restricted final : private SemanticSubsystem
    {
        friend class CallBuilder;

    private:
        struct
        {
            std::string id;
            SourceLocation id_loc;
        } method_call_draft_;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void update_method_call_draft(const char *id, SourceLocation id_loc);
        [[nodiscard]] const Expr *build_call_consuming(
            const Expr *callable_lvalue, ExprList *&param_list, SourceLocation call_loc);
        [[nodiscard]] const Expr *build_method_call_consuming(
            const Expr *callable_lvalue, ExprList *&elist, SourceLocation call_loc);
        [[nodiscard]] const Expr *build_iife_call_consuming(
            const FuncSymbol *func_symbol, ExprList *&elist, SourceLocation call_loc);

        static void delete_expr_list(ExprList *&param_list);
    };

    Restricted DISPATCH_TARGET;

    explicit CallBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(update_method_call_draft);
    DISPATCH_SLAVE_METHOD_CALL(build_call_consuming);
    DISPATCH_SLAVE_METHOD_CALL(build_iife_call_consuming);
    DISPATCH_SLAVE_METHOD_CALL(build_method_call_consuming);
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
            SourceLocation loc;
            std::vector<Parameter> parameter_list;

            void reset() { id = std::string(), loc = k_no_loc, parameter_list.clear(); }
        } function_draft_;

        u32 next_function_address_;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void update_function_draft(SourceLocation function_loc);
        void update_function_draft(const std::string &id, SourceLocation function_loc);
        void collect_function_parameter(const std::string &id, SourceLocation id_loc);
        [[nodiscard]] const Expr *build_program_function(
            const FuncSymbol *func_symbol, SourceLocation result_loc);
        [[nodiscard]] const FuncSymbol *build_program_function_entry(SourceLocation entry_loc);
        [[nodiscard]] const FuncSymbol *build_program_function_exit(BlockSourceLocation block_loc);

        void register_function_parameters();
        [[nodiscard]] bool validate_funcdef_name(
            const std::string &func_name, SourceLocation funcname_loc);
        [[nodiscard]] bool validate_formal_param_name(const Parameter &param);
    };

    Restricted DISPATCH_TARGET;

    explicit FunctionBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(update_function_draft);
    DISPATCH_SLAVE_METHOD_CALL(collect_function_parameter);
    DISPATCH_SLAVE_METHOD_CALL(build_program_function);
    DISPATCH_SLAVE_METHOD_CALL(build_program_function_entry);
    DISPATCH_SLAVE_METHOD_CALL(build_program_function_exit);
    DISPATCH_DEFINE_HANDLER_END();
};

class TableAccessBuilder
{
    friend class SemanticSystem;

private:
    class Restricted final : private SemanticSubsystem
    {
        friend class TableAccessBuilder;

    private:
        [[nodiscard]] const Expr *build_member_access(
            const Expr *lvalue,
            const char *member_id,
            SourceLocation member_id_loc,
            SourceLocation result_loc);
        [[nodiscard]] const Expr *build_index_access(
            const Expr *lvalue, const Expr *index, SourceLocation result_loc);

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;
    };

    Restricted DISPATCH_TARGET;

    explicit TableAccessBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(build_member_access);
    DISPATCH_SLAVE_METHOD_CALL(build_index_access);
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
    elist = nullptr;
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
    ExprList *&elist,
    const SourceLocation table_list_loc)
{
    DEBUG_SMART_ASSERT(!!elist);
    auto *const qh = quad_handler_; // Short alias for readability.

    const NewTableExpr *const new_table_expr = expr_maker_->make_new_table_expr(table_list_loc);
    qh->emit_next(ir::Opcode::TABLECREATE, new_table_expr, nullptr, nullptr, table_list_loc);

    // Emit exprlist's items.
    u32 list_index = 0;
    for (auto expr_it = elist->crbegin(); expr_it != elist->crend(); ++expr_it)
    {
        const Expr *const list_item = *expr_it;
        const SourceLocation list_item_loc = list_item->loc;
        const Expr *const idx_expr = expr_maker_->make_const_int_expr(list_item_loc, list_index++);
        qh->emit_next(ir::Opcode::TABLESETELEM, new_table_expr, idx_expr, list_item, list_item_loc);
    }

    // Delete elist after use — it must not be used again
    AggregateBuilder::Restricted::delete_expr_list(elist);

    return new_table_expr;
}

inline const Expr *
AggregateBuilder::Restricted::build_table_dict_consuming(
    DictList *&dlist,
    const SourceLocation table_dict_loc)
{
    DEBUG_SMART_ASSERT(!!dlist);
    auto *const qh = quad_handler_; // Short alias for readability.

    const Expr *const new_table_expr = expr_maker_->make_new_table_expr(table_dict_loc);
    qh->emit_next(ir::Opcode::TABLECREATE, new_table_expr, nullptr, nullptr, table_dict_loc);

    // Emit dict's items.
    for (auto it = dlist->crbegin(); it != dlist->crend(); ++it)
    {
        const Expr *const key = (*it)->first;
        const Expr *const value = (*it)->second;
        const SourceLocation pair_loc = merge(key->loc, value->loc);
        qh->emit_next(ir::Opcode::TABLESETELEM, new_table_expr, key, value, pair_loc);
    }

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
    // If here. Its Lvalue and all lvalues have symbols.
    if (lvalue->type == Expr::Type::LIBRARY_FUNCTION)
    {
        const auto *const func_symbol = static_cast<const LibFuncExpr *>(lvalue)->func_symbol;
        dr_->report_assign_to_libfunc(func_symbol->name, assign_loc);
        return false;
    }
    if (lvalue->type == Expr::Type::PROGRAM_FUNCTION)
    {
        const auto *const func_symbol = static_cast<const ProgFuncExpr *>(lvalue)->func_symbol;
        dr_->report_assign_to_func(func_symbol->name, assign_loc, func_symbol->loc);
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
    quad_handler_->emit_next(ir::Opcode::TABLESETELEM, rvalue, ti, ti->index, result_loc);
    const Expr *temp_var = ss_bridge_->materialize_if_table_item(lvalue); // !CERTAIN EMIT!
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
    quad_handler_->emit_next(ir::Opcode::ASSIGN, lvalue, rvalue, nullptr, result_loc);
    quad_handler_->emit_next(ir::Opcode::ASSIGN, temp, lvalue, nullptr, result_loc);
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
        static_assert([]() { return false; }(), "build_inc_dec(): Unknown OpVariant");
}

template<typename Policy>
const Expr *
AssignBuilder::Restricted::handle_pre_inc_dec(const Expr *lvalue, const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> ||
                  std::is_same_v<Policy, DecPolicy>, "Expected INC or DEC policy");
    auto *const qh = quad_handler_; // Short alias for readability.

    const Expr *result = nullptr;
    if (lvalue->type == Expr::Type::TABLE_ITEM)
    {
        const auto *const ti_lvalue = static_cast<const TableItemExpr *>(lvalue);
        result = ss_bridge_->materialize_if_table_item(ti_lvalue); // EMITS!
        qh->emit_next(Policy::opc, result, result, &k_static_int_1_expr, result_loc);
        qh->emit_next(ir::Opcode::TABLESETELEM, result, ti_lvalue, ti_lvalue->index, result_loc);
    }
    else
    {
        // TODO: HOOK: After you implemented logic to make assignment aware of if its happening,
        // inside a function parameter list (TODO 52), create this new arithmetic_expr (new temp)
        // only if inside assignment. NOTE! ONLY ENABLE THIS OPTIMIZATION IFF optimization options is passed.
        // DO NOT make it standard behavior.. you may get fucked in examination :D
        result = expr_maker_->make_arithmetic_expr(result_loc);
        qh->emit_next(Policy::opc, lvalue, lvalue, &k_static_int_1_expr, result_loc);
        qh->emit_next(ir::Opcode::ASSIGN, result, lvalue, nullptr, result_loc);
    }
    return DEBUG_REQUIRE_PTR(result); // Check because we initialized with nullptr.
}

template<typename Policy>
const Expr *
AssignBuilder::Restricted::handle_post_inc_dec(const Expr *lvalue, const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> || std::is_same_v<Policy, DecPolicy>);
    auto *const qh = quad_handler_; // Short alias for readability.

    const Expr *result = expr_maker_->make_variable_expr(result_loc, parse_ctx_->new_temp());
    if (lvalue->type == Expr::Type::TABLE_ITEM)
    {
        const auto *const ti_lvalue = static_cast<const TableItemExpr *>(lvalue);
        const Expr *ti = ss_bridge_->materialize_if_table_item(lvalue);
        qh->emit_next(ir::Opcode::ASSIGN, result, ti, nullptr, result_loc);
        qh->emit_next(Policy::opc, ti, ti, &k_static_int_1_expr, result_loc);
        qh->emit_next(ir::Opcode::TABLESETELEM, ti, ti_lvalue, ti_lvalue->index, result_loc);
    }
    else
    {
        qh->emit_next(ir::Opcode::ASSIGN, result, lvalue, nullptr, result_loc);
        qh->emit_next(Policy::opc, lvalue, lvalue, &k_static_int_1_expr, result_loc);
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
    if (!validate_arithmetic_expr(ir::Opcode::UMINUS, expr, OperandSide::UNARY))
        return nullptr;
    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::UMINUS>(result_loc, expr))
        return optimized;

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next(ir::Opcode::UMINUS, arithmetic_expr, expr, nullptr, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::Restricted::build_arithmetic(
    const ir::Opcode opc,
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (!validate_arithmetic_expr(opc, lhs, OperandSide::LEFT)) return nullptr;
    if (!validate_arithmetic_expr(opc, rhs, OperandSide::RIGHT)) return nullptr;
    if (!validate_possible_division(opc, rhs, result_loc)) return nullptr;

    if (const auto optimized = this->try_optimize_arithmetic_expr(opc, lhs, rhs, result_loc))
        return optimized;

    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next(opc, arithmetic_expr, lhs, rhs, result_loc);
    return arithmetic_expr;
}

inline const Expr *
BasicBuilder::Restricted::build_relational(
    const ir::Opcode opc,
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (!validate_relational_expr(opc, lhs, OperandSide::LEFT)) return nullptr;
    if (!validate_relational_expr(opc, rhs, OperandSide::RIGHT)) return nullptr;

    if (const auto optimized = this->try_optimize_relational_expr(opc, lhs, rhs, result_loc))
        return optimized;

    const BoolExpr *result_expr = expr_maker_->make_bool_expr(result_loc);
    result_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(opc, nullptr, lhs, rhs, result_loc);
    result_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, result_loc);
    return result_expr;
}

inline const Expr *
BasicBuilder::Restricted::build_logical_not(const Expr *expr, const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr, SemUtils::is_bool_or_const_bool_expr(expr));

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::NOT>(result_loc, expr))
        return optimized;

    // Sanity check, CONST_BOOL must be consumed by the optimizer.
    DEBUG_SMART_ASSERT(expr->type == Expr::Type::BOOL_EXPR);
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
        SemUtils::is_bool_or_const_bool_expr(lhs),
        SemUtils::is_bool_or_const_bool_expr(rhs)
    );

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::AND>(result_loc, lhs, rhs))
        return optimized;
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
        SemUtils::is_bool_or_const_bool_expr(lhs),
        SemUtils::is_bool_or_const_bool_expr(rhs)
    );

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::OR>(result_loc, lhs, rhs))
        return optimized;
    return build_short_circuit_bool_expr<OrShortCircuitPolicy>(lhs, rhs, result_loc);
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
BasicBuilder::Restricted::validate_arithmetic_expr(
    const ir::Opcode opc,
    const Expr *expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (SemUtils::is_arithmetic_convertible_expr(expr))
        return true;

    if (SemUtils::is_binary_arithmetic_opcode(opc))
        dr_->report_arith_op_nonarith_operand(op_side, SemUtils::arith_op_str(opc), expr->type,
                                              expr->loc);
    else if (opc == ir::Opcode::UMINUS)
        dr_->report_uminus_nonarith_operand(expr->type, expr->loc);
    else
        throw std::logic_error(ATTACH_CONTEXT(
            "Expected arithmetic ir::Opcode (bin arith or uminus)"));
    return false;
}

inline bool
BasicBuilder::Restricted::validate_relational_expr(
    const ir::Opcode opc,
    const Expr *const expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr,);
    // In Alpha everything is convertible to bool.
    // And operators == and != convert their operands to bool.
    if (SemUtils::is_relational_equality_iropcode(opc))
        return true;
    // If here relational operator is:  < <= > >=
    if (SemUtils::is_arithmetic_convertible_expr(expr))
        return true;
    dr_->report_rel_op_nonarith_operand(op_side, SemUtils::relop_str(opc), expr->type, expr->loc);
    return false;
}

inline bool
BasicBuilder::Restricted::validate_possible_division(
    const ir::Opcode iropcode,
    const Expr *const rhs,
    const SourceLocation division_loc)
{
    if (iropcode != ir::Opcode::DIV && iropcode != ir::Opcode::MOD)
        return true;
    if (!SemUtils::is_const_0(rhs))
        return true;
    dr_->report_division_by_zero(division_loc);
    return false;
}

inline const Expr *
BasicBuilder::Restricted::try_optimize_arithmetic_expr(
    const ir::Opcode opc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    using Op = ir::Opcode;
    switch (opc)
    {
    case Op::ADD: return expr_optimizer_->try_optimize<Op::ADD>(result_loc, lhs, rhs);
    case Op::SUB: return expr_optimizer_->try_optimize<Op::SUB>(result_loc, lhs, rhs);
    case Op::MUL: return expr_optimizer_->try_optimize<Op::MUL>(result_loc, lhs, rhs);
    case Op::DIV: return expr_optimizer_->try_optimize<Op::DIV>(result_loc, lhs, rhs);
    case Op::MOD: return expr_optimizer_->try_optimize<Op::MOD>(result_loc, lhs, rhs);
    default: [[unlikely]] UNREACHABLE(FMT::format("Unexpected opcode: {}", static_cast<int>(opc)));
    }
}

inline const Expr *
BasicBuilder::Restricted::try_optimize_relational_expr(
    const ir::Opcode opc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    using Op = ir::Opcode;
    switch (opc)
    {
    case Op::IF_EQ: return expr_optimizer_->try_optimize<Op::IF_EQ>(result_loc, lhs, rhs);
    case Op::IF_NEQ: return expr_optimizer_->try_optimize<Op::IF_NEQ>(result_loc, lhs, rhs);
    case Op::IF_LT: return expr_optimizer_->try_optimize<Op::IF_LT>(result_loc, lhs, rhs);
    case Op::IF_LTE: return expr_optimizer_->try_optimize<Op::IF_LTE>(result_loc, lhs, rhs);
    case Op::IF_GT: return expr_optimizer_->try_optimize<Op::IF_GT>(result_loc, lhs, rhs);
    case Op::IF_GTE: return expr_optimizer_->try_optimize<Op::IF_GTE>(result_loc, lhs, rhs);
    default: [[unlikely]] UNREACHABLE(FMT::format("Unexpected opcode: {}", static_cast<int>(opc)));
    }
}

inline void
BasicBuilder::Restricted::warn_if_lossy_conversion_int_to_float(
    const AlphaInt value,
    const SourceLocation conversion_loc)
{
    if (!utils::is_lossless_int_to_float<AlphaFloat>(value))
        dr_->report_implicit_int_to_float_loss(conversion_loc);
}

inline void
CallBuilder::Restricted::update_method_call_draft(
    const char *const id,
    const SourceLocation id_loc)
{
    method_call_draft_.id = id;
    method_call_draft_.id_loc = id_loc;
}

inline const Expr *
CallBuilder::Restricted::build_call_consuming(
    const Expr *const callable_lvalue,
    ExprList *&param_list,
    const SourceLocation call_loc)
{
    DEBUG_SMART_ASSERT(!!callable_lvalue, !!param_list);

    const Expr *func_expr = ss_bridge_->materialize_if_table_item(callable_lvalue);
    for (const Expr *e: *param_list)
        quad_handler_->emit_next(ir::Opcode::PARAM, nullptr, e, nullptr, e->loc);

    quad_handler_->emit_next(ir::Opcode::CALL, nullptr, func_expr, nullptr, call_loc);

    const Expr *getretval_expr = expr_maker_->make_variable_expr(call_loc, parse_ctx_->new_temp());
    quad_handler_->emit_next(ir::Opcode::GETRETVAL, getretval_expr, nullptr, nullptr, call_loc);

    CallBuilder::Restricted::delete_expr_list(param_list);
    return getretval_expr;
}

inline const Expr *
CallBuilder::Restricted::build_method_call_consuming(
    const Expr *const callable_lvalue, ExprList *&elist, const SourceLocation call_loc)
{
    // TODO: Make ExprList (elist) and DictList(dlist) self-manageable (either methods)
    // or using ADL.
    auto lvalue = ss_bridge_->materialize_if_table_item(callable_lvalue);
    elist->push_back(lvalue);

    const Expr *const method_index = expr_maker_->make_const_string_expr(
        method_call_draft_.id_loc, method_call_draft_.id.c_str());
    const Expr *const hosting_var = expr_maker_->make_table_item_expr(
        k_no_loc, lvalue, method_index);

    lvalue = ss_bridge_->materialize_if_table_item(hosting_var);
    return build_call_consuming(lvalue, elist, call_loc);
}

inline const Expr *
CallBuilder::Restricted::build_iife_call_consuming(
    const FuncSymbol *const func_symbol, ExprList *&elist, const SourceLocation call_loc)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    const auto *const prog_func_expr = expr_maker_->make_prog_func_expr(call_loc, func_symbol);
    return build_call_consuming(prog_func_expr, elist, call_loc);
}

inline void CallBuilder::Restricted::delete_expr_list(ExprList *&param_list)
{
    delete param_list;
    param_list = nullptr;
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
FunctionBuilder::Restricted::update_function_draft(
    const SourceLocation function_loc)
{
    update_function_draft(parse_ctx_->name_generator.new_anonymous(), function_loc);
}

inline void
FunctionBuilder::Restricted::update_function_draft(
    const std::string &id,
    const SourceLocation function_loc)
{
    function_draft_.id = id;
    function_draft_.loc = function_loc;
    parse_ctx_->space_handler.enter_space();
}

inline void
FunctionBuilder::Restricted::collect_function_parameter(
    const std::string &id,
    const SourceLocation id_loc) { function_draft_.parameter_list.emplace_back(id, id_loc); }

inline void
FunctionBuilder::Restricted::register_function_parameters()
{
    constexpr auto space = VarSymbol::Space::FORMAL_ARGUMENT;
    DEBUG_SMART_ASSERT(
        parse_ctx_->space_handler.space() == VarSymbol::Space::FORMAL_ARGUMENT
    );

    for (const Parameter &p: function_draft_.parameter_list)
        if (validate_formal_param_name(p))
            symbol_table_->insert_variable(
                p.name,
                parse_ctx_->scope_handler.scope(),
                VarSymbol::Type::FORMAL_ARGUMENT,
                space,
                parse_ctx_->space_handler.next_offset(),
                p.loc
            );
}

inline bool
FunctionBuilder::Restricted::validate_funcdef_name(
    const std::string &func_name,
    const SourceLocation funcname_loc)
{
    if (symbol_table_->is_libfunc_name(func_name))
    {
        dr_->report_redefinition_of_libfunc(func_name, funcname_loc);
        return false;
    }

    const auto curr_scope = parse_ctx_->scope_handler.scope();
    if (const Symbol *const found_symbol = symbol_table_->lookup_local(func_name, curr_scope))
    {
        if (found_symbol->is_function())
        {
            dr_->report_redefinition_of_func(func_name, funcname_loc, found_symbol->loc);
            return false;
        }
        if (found_symbol->is_variable())
        {
            dr_->report_var_redefined_as_func(func_name, funcname_loc, found_symbol->loc);
            return false;
        }
    }
    return true;
}

inline bool
FunctionBuilder::Restricted::validate_formal_param_name(const Parameter &param)
{
    // Library‐function conflict
    if (symbol_table_->is_libfunc_name(param.name))
    {
        dr_->report_libfunc_redefined_as_formal_parameter(param.name, param.loc);
        return false;
    }

    const auto curr_scope = parse_ctx_->scope_handler.scope();
    if (const Symbol *const formal_symbol = symbol_table_->lookup_local(param.name, curr_scope))
    {
        // Parameter should produce name conflicts only with themselves.
        DEBUG_SMART_ASSERT(
            !!dynamic_cast<const VarSymbol *>(formal_symbol), // non-nullptr == valid conversion
            formal_symbol->is_variable(),
            formal_symbol->type == VarSymbol::Type::FORMAL_ARGUMENT
        );
        dr_->report_redefinition_of_formal_parameter(param.name, param.loc, formal_symbol->loc);
        return false;
    }
    return true;
}

inline const Expr *
FunctionBuilder::Restricted::build_program_function(
    const FuncSymbol *const func_symbol,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    return expr_maker_->make_prog_func_expr(result_loc, func_symbol);
}

/// Handles a function signature’s prefix + argument list.
///
/// If a name conflict is detected, we still need to call
/// enter_function() (to keep our frame‐stack balanced), but
/// we must *NOT* back-patch the local-variable count,
/// or we’ll end up polluting the original function’s frame with
/// local_variable_count from the redefinition. TODO: DO WE POLLUTE CURRENTLY?
inline const FuncSymbol *
FunctionBuilder::Restricted::build_program_function_entry(const SourceLocation entry_loc)
{
    const auto &func_name = function_draft_.id; // Local alias for readability
    const auto func_loc = function_draft_.loc;  // Local alias for readability.
    const bool validated_funcname = validate_funcdef_name(func_name, func_loc);

    // TODO: Why is this needed (observe generated IR)
    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, func_loc);
    const FuncSymbol *func_symbol = nullptr;
    if (validated_funcname)
    {
        func_symbol = symbol_table_->insert_function(
            func_name,
            parse_ctx_->scope_handler.scope(),
            next_function_address_,
            function_draft_.parameter_list,
            func_loc
        );

        quad_handler_->emit_next(
            ir::Opcode::FUNCSTART,
            nullptr,
            expr_maker_->make_prog_func_expr(entry_loc, func_symbol),
            nullptr,
            func_loc
        );
    }
    DEBUG_SMART_ASSERT(utils::logical_xnor(validated_funcname, !!func_symbol)); // Sanity check

    // TODO: Wtf is this label of jump? IF YOU FIND OUT... REMEMBER TO RENAME ALL INSTANCES or string/comment/text of `label_of_jumps` (case-insensitive)
    const u32 label_of_jump = quad_handler_->next_quad_label();
    parse_ctx_->func_ctx_handler.enter_function(func_name, func_loc, func_symbol, label_of_jump);
    register_function_parameters();
    function_draft_.reset(); // Mandatory to support nested functions in the upcoming func-block.
    parse_ctx_->space_handler.enter_space(); // New var space -- must be after param registration.

    return func_symbol;
}

//TODO reallocate? Am I even needed?

inline const FuncSymbol *
FunctionBuilder::Restricted::build_program_function_exit(const BlockSourceLocation block_loc)
{
    quad_handler_->patch_list(
        parse_ctx_->func_ctx_handler.return_list(), quad_handler_->next_quad_label());

    const auto fbi = parse_ctx_->func_ctx_handler.exit_function();
    if (!!fbi.func_symbol)
    {
        fbi.func_symbol->stackframe_slot_count = fbi.local_var_count;

        quad_handler_->emit_next(
            ir::Opcode::FUNCEND,
            nullptr,
            expr_maker_->make_prog_func_expr(block_loc.end, fbi.func_symbol),
            nullptr,
            block_loc.end);
    }
    quad_handler_->patch_quad(fbi.label_to_jump, quad_handler_->next_quad_label());
    parse_ctx_->space_handler.exit_space();

    return fbi.func_symbol;
}

inline const Expr *
TableAccessBuilder::Restricted::build_member_access(
    const Expr *const lvalue,
    const char *const member_id,
    const SourceLocation member_id_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!member_id);
    const Expr *const normalized_lvalue = ss_bridge_->materialize_if_table_item(lvalue);
    const Expr *const index = expr_maker_->make_const_string_expr(member_id_loc, member_id);
    return expr_maker_->make_table_item_expr(result_loc, normalized_lvalue, index);
}

inline const Expr *
TableAccessBuilder::Restricted::build_index_access(
    const Expr *const lvalue,
    const Expr *const index,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!index);
    const Expr *const normalized_lvalue = ss_bridge_->materialize_if_table_item(lvalue);
    return expr_maker_->make_table_item_expr(result_loc, normalized_lvalue, index);
}
} // namespace alpha
#endif // EXPR_BUILDERS_HPP
