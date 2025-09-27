#ifndef EXPR_BUILDERS_HPP
#define EXPR_BUILDERS_HPP

#include <functional>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include <parser/ir_opcode.gen.hpp>
#include  <support/misc_tools.hpp>
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
        struct TableLiteralCtx
        {
            std::size_t list_index = 0; // Only used for ExprList. NOT DictList!
            const NewTableExpr *const host_expr;

            explicit TableLiteralCtx(const NewTableExpr *const new_table_expr)
                : host_expr(new_table_expr) {}
        };

        struct
        {
            // A stack is required because we might have nested aggregates.
            std::stack<TableLiteralCtx, std::vector<TableLiteralCtx>> table_literal_stack;
        } draft_;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        // List related (candidate for submodule)
        void mark_temp_checkpoint();
        [[nodiscard]] static ExprList *build_expr_list();
        [[nodiscard]] ExprList *build_expr_list(const Expr *head_expr);
        #ifdef CYA_MODE
        [[nodiscard]] ExprList *extend_expr_list(ExprList *elist, const Expr *next);
        #else
        void commit_table_element(const Expr *table_elem);
        #endif
        static void delete_expr_list(ExprList *elist);
        static void consume_expr_list(ExprList *elist) { delete_expr_list(elist); }

        // Dict related (candidate for submodule)
        void begin_dict_entry();
        void end_dict_entry();
        [[nodiscard]] const ExprPair *build_dict_entry(const Expr *key, const Expr *val);
        [[nodiscard]] static DictList *build_dict_list();
        [[nodiscard]] DictList *build_dict_list(const ExprPair *head_pair);
        [[nodiscard]] DictList *extend_dict_list(DictList *dlist, const ExprPair *next_pair);
        static void delete_dict_list(DictList *dlist);
        static void consume_dict_list(DictList *dlist) { delete_dict_list(dlist); }

        void initiate_table_literal(SourceLocation table_list_loc);

        #ifdef CYA_MODE
        [[nodiscard]] const Expr *build_table_list_consuming(
            ExprList *elist, SourceLocation table_list_loc);
        [[nodiscard]] const Expr *build_table_dict_consuming(
            DictList *dlist, SourceLocation table_dict_loc);
        #else
        [[nodiscard]] const Expr *extract_table_literal_consuming(ExprList *elist);
        [[nodiscard]] const Expr *extract_table_literal_consuming(DictList *dlist);
        #endif

        template<typename ListT>
        [[nodiscard]] const Expr *extract_table_literal_consuming_impl(
            ListT *list, void (*deleter)(ListT *));
    };

    Restricted DISPATCH_TARGET;

    explicit AggregateBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(mark_temp_checkpoint);
    DISPATCH_SLAVE_METHOD_CALL(build_expr_list);
    DISPATCH_SLAVE_METHOD_CALL(begin_dict_entry);
    DISPATCH_SLAVE_METHOD_CALL(end_dict_entry);
    DISPATCH_SLAVE_METHOD_CALL(build_dict_entry);
    DISPATCH_SLAVE_METHOD_CALL(build_dict_list);
    #ifdef CYA_MODE
    DISPATCH_SLAVE_METHOD_CALL(extend_expr_list);
    #else
    DISPATCH_SLAVE_METHOD_CALL(commit_table_element);
    #endif
    DISPATCH_SLAVE_METHOD_CALL(extend_dict_list);
    DISPATCH_SLAVE_METHOD_CALL(consume_expr_list);
    DISPATCH_SLAVE_METHOD_CALL(consume_dict_list);
    DISPATCH_SLAVE_METHOD_CALL(initiate_table_literal);
    #ifdef CYA_MODE
    DISPATCH_SLAVE_METHOD_CALL(build_table_list_consuming);
    DISPATCH_SLAVE_METHOD_CALL(build_table_dict_consuming);
    #else
    DISPATCH_SLAVE_METHOD_CALL(extract_table_literal_consuming);
    #endif
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
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_pre_inc(const Expr *expr, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_post_inc(const Expr *expr, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_pre_dec(const Expr *expr, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_post_dec(const Expr *expr, SourceLocation result_loc);

        [[nodiscard]] static bool is_direct_target_expr(const Expr *expr) noexcept;
        [[nodiscard]] bool assignment_requires_temp() const;
        [[nodiscard]] bool validate_assignment(const Expr *lhs, SourceLocation assign_loc);
        [[nodiscard]] bool try_record_const_expr(const Expr *lvalue, const Expr *rvalue);
        [[nodiscard]] const Expr *handle_direct_assignment(
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *handle_table_item_assignment(
            const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

        template<typename Policy>
        [[nodiscard]] const Expr *handle_pre_inc_dec(const Expr *expr, SourceLocation result_loc);
        template<typename Policy>
        [[nodiscard]] const Expr *handle_post_inc_dec(const Expr *expr, SourceLocation result_loc);
        template<OpVariant op_variant, typename Policy>
        [[nodiscard]] const Expr *build_inc_dec(const Expr *expr, SourceLocation result_loc);
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
        bool fold_static_bools;
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

        [[nodiscard]] const Expr *prepare_logical_operand_expr(const Expr *expr);
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

        [[nodiscard]] const Expr *normalize_to_bool_expr(const Expr *expr);
        [[nodiscard]] bool validate_arithmetic_expr(
            ir::Opcode opc, const Expr *expr, OperandSide op_side);
        [[nodiscard]] bool validate_relational_expr(
            ir::Opcode opc, const Expr *expr, OperandSide op_side);
        [[nodiscard]] bool validate_possible_division(
            ir::Opcode opc, const Expr *rhs, SourceLocation division_loc);

        // When I built the compile-time call dispatcher for Bison, I didn’t add support for template args.
        // Later, I made the optimizer fully templated. Rather than making it runtime-based,
        // I use a clean runtime to compile-time dispatcher for expr_optimizer's try_optimize()
        // Only arithmetic and relational builders take ir::Opcode as a runtime arg,
        // since they share logic with the opcode being the only varying part.
        [[nodiscard]] const Expr *try_optimize_arithmetic_expr(
            ir::Opcode opc, const Expr *&lhs, const Expr *&rhs, SourceLocation result_loc);
        [[nodiscard]] const Expr *try_optimize_relational_expr(
            ir::Opcode opc, const Expr *&lhs, const Expr *&rhs, SourceLocation result_loc);

        void warn_if_lossy_conversion_int_to_float(AlphaInt value, SourceLocation conversion_loc);
    };

    Restricted DISPATCH_TARGET;

    BasicBuilder(Options &&options, const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(prepare_logical_operand_expr);
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

        void stage_call_space();
        void retire_call_space();
        #ifndef CYa_MODE
        void commit_call_argument(const Expr* call_arg);
        #endif

        [[nodiscard]] const Expr *build_call_consuming(
            const Expr *callable_lvalue, ExprList *arg_list, SourceLocation call_loc,
            const Expr *method = nullptr);
        [[nodiscard]] const Expr *build_method_call_consuming(
            const Expr *callable_lvalue, ExprList *arg_list, SourceLocation call_loc);
        [[nodiscard]] const Expr *build_iife_call_consuming(
            const FuncSymbol *func_symbol, ExprList *arg_list, SourceLocation call_loc);

        void check_for_argument_mismatch(
            const Expr *callable_lvalue, const ExprList *param_list, SourceLocation call_loc);

        static void delete_expr_list(ExprList *param_list);
    };

    Restricted DISPATCH_TARGET;

    explicit CallBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(update_method_call_draft);
    DISPATCH_SLAVE_METHOD_CALL(stage_call_space);
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
            std::vector<Parameter> parameter_list;

            void reset() { id = std::string(), parameter_list.clear(); }
        } function_draft_;

        u32 next_function_address_ = k_first_function_address;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void update_function_draft();
        void update_function_draft(const std::string &id);
        void collect_function_parameter(const std::string &id, SourceLocation id_loc);
        [[nodiscard]] const Expr *forward_program_function(
            const FuncSymbol *func_symbol, SourceLocation result_loc);
        [[nodiscard]] const FuncSymbol *build_program_function_entry(
            SourceLocation func_signature_loc);
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
    DISPATCH_SLAVE_METHOD_CALL(forward_program_function);
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
            const Expr *base,
            const char *member_id,
            SourceLocation member_id_loc,
            SourceLocation access_loc);
        [[nodiscard]] const Expr *build_subscript_access(
            const Expr *base, const Expr *subscript, SourceLocation access_loc);

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;
    };

    Restricted DISPATCH_TARGET;

    explicit TableAccessBuilder(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(build_member_access);
    DISPATCH_SLAVE_METHOD_CALL(build_subscript_access);
    DISPATCH_DEFINE_HANDLER_END();
};

inline void
AggregateBuilder::Restricted::mark_temp_checkpoint()
{
    #ifndef CYA_MODE
    if (draft_.table_literal_stack.empty())
        parse_ctx_->temp_ctx_handler.push_checkpoint();
    #endif
}

inline ExprList *
AggregateBuilder::Restricted::build_expr_list() { return new ExprList(); }

inline ExprList *
AggregateBuilder::Restricted::build_expr_list(const Expr *const head_expr)
{
    DEBUG_SMART_ASSERT(!!head_expr);
    return commit_table_elem(build_expr_list(), head_expr);
}

inline void
AggregateBuilder::Restricted::delete_expr_list(ExprList *elist)
{
    // Note: Do NOT delete the expressions in ExprList -- those are handler by ExprMaker.
    delete elist;
}


inline void
AggregateBuilder::Restricted::begin_dict_entry()
{
    parse_ctx_->aggregate_ctx_handler.enter_dict_entry();
}

inline void
AggregateBuilder::Restricted::end_dict_entry()
{
    parse_ctx_->aggregate_ctx_handler.exit_dict_entry();
}

inline DictList *
AggregateBuilder::Restricted::build_dict_list() { return new DictList(); }

inline DictList *
AggregateBuilder::Restricted::build_dict_list(const ExprPair *const head_pair)
{
    DEBUG_SMART_ASSERT(!!head_pair);
    return extend_dict_list(build_dict_list(), head_pair);
}
} // namespace alpha
#endif // EXPR_BUILDERS_HPP
