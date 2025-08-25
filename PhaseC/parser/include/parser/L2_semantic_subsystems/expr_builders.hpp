#ifndef EXPR_BUILDERS_HPP
#define EXPR_BUILDERS_HPP

#include <functional>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include <parser/ir_opcode.gen.hpp>
#include  <utils/misc.hpp>
#include "semantic_subsystem.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/expr_optimizer.hpp"
#include "L3_ir_infra/quad_handler.hpp"
#include "parser/semantic_utils.hpp"

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
        struct
        {
            ExprList elist;
            DictList dlist;
        }draft_;

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

        template<OpVariant op_variant,typename Policy>
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
} // namespace alpha
#endif // EXPR_BUILDERS_HPP
