#ifndef EXPR_BUILDERS_HPP
#define EXPR_BUILDERS_HPP

#include <functional>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include <parser/ir_opcode.gen.hpp>
#include  <support/misc_tools.hpp>
#include "semantic_subsystem.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include "core/expr_maker.hpp"
#include "core/expr_optimizer.hpp"
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
        using DictList = std::vector<const ExprPair *>;

        struct TableLiteralInfo
        {
            std::size_t list_index = 0; // Only used for ExprList. NOT DictList!
            const NewTableExpr *const host_expr;
            const LabelID host_quad_label;

            TableLiteralInfo(const NewTableExpr *new_table_expr, LabelID host_quad_label);
        };

        struct
        {
            // A stack is required because we can have nested aggregates.
            VectorStack<TableLiteralInfo> table_literal_stack;
        } draft_;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void init_table_literal();
        [[nodiscard]] const Expr *finalize_table_literal(SourceLocation table_loc);

        // Dict related (candidate for submodule)
        void begin_dict_entry();
        void end_dict_entry();
        void commit_dict_element(const Expr *key, const Expr *value, SourceLocation dict_elem_loc);
    };

    // Accessors exists to insulate call sites from the DISPATCH_TARGET macro
    // and to make the intended access point to Restricted state explicit.
    Restricted DISPATCH_TARGET;
    Restricted &restricted() noexcept { return DISPATCH_TARGET; }
    const Restricted &restricted() const noexcept { return DISPATCH_TARGET; }

    explicit AggregateBuilder(const SemanticSystemServices &ss_services);

    // Defined outside Restricted, so it can be accessed by SemanticSystem's generalized expr collector
    void commit_list_element(const Expr *list_elem);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(init_table_literal);
    DISPATCH_SLAVE_METHOD_CALL(finalize_table_literal);
    DISPATCH_SLAVE_METHOD_CALL(begin_dict_entry);
    DISPATCH_SLAVE_METHOD_CALL(end_dict_entry);
    DISPATCH_SLAVE_METHOD_CALL(commit_dict_element);
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
        [[nodiscard]] const Expr *handle_pre_inc_dec(const Expr *lvalue, SourceLocation result_loc);
        template<typename Policy>
        [[nodiscard]] const Expr *
        handle_post_inc_dec(const Expr *lvalue, SourceLocation result_loc);
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
        [[nodiscard]] const Expr *build_uminus(
            const Expr *expr, SourceLocation uminus_loc, SourceLocation result_loc);
        [[nodiscard]] const Expr *build_arithmetic(
            ir::Opcode opc,
            const Expr *lhs,
            const Expr *rhs,
            SourceLocation arith_op_loc,
            SourceLocation result_loc);
        [[nodiscard]] const Expr *build_relational(
            ir::Opcode opc,
            const Expr *lhs,
            const Expr *rhs,
            SourceLocation operator_loc,
            SourceLocation result_loc);
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
            OperandSide op_side, ir::Opcode opc, const Expr *expr, SourceLocation arith_op_loc);
        [[nodiscard]] bool validate_arithmetic_operation(
            ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation arith_op_loc);
        [[nodiscard]] bool validate_relational_operation(
            ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation operator_loc);
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
        struct MethodInfo
        {
            const StringSpan id;
            const SourceLocation id_loc;
        };

        struct CallInfo
        {
            using ArgStack = VectorStack<const Expr *>;

            std::optional<MethodInfo> pending_method_info;
            ArgStack arguments;

            CallInfo();
            explicit CallInfo(MethodInfo method_info);
        };

        struct
        {
            // A stack is required because we can have nested calls.
            VectorStack<CallInfo> call_info_stack;
            std::optional<MethodInfo> immediate_method_info;
        } draft_;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void update_method_call_draft(StringSpan method_id, SourceLocation method_id_loc);

        void init_call();
        void finalize_call();

        void check_for_argument_mismatch(
            const Expr *callable_lvalue,
            const CallInfo::ArgStack &arg_stack,
            SourceLocation call_loc);

        [[nodiscard]] const Expr *build_call_consuming(
            const Expr *callable,
            SourceLocation call_loc,
            const ConstStringExpr *method_name = nullptr);
        [[nodiscard]] const Expr *build_method_call_consuming(
            const Expr *method_host, SourceLocation call_loc);
        [[nodiscard]] const Expr *build_iife_call_consuming(
            const ProgFuncSymbol *func_symbol, SourceLocation call_loc);
    };

    Restricted DISPATCH_TARGET;
    Restricted &restricted() noexcept { return DISPATCH_TARGET; }
    const Restricted &restricted() const noexcept { return DISPATCH_TARGET; }

    explicit CallBuilder(const SemanticSystemServices &ss_services);

    void commit_call_argument(const Expr *call_arg);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(update_method_call_draft);
    DISPATCH_SLAVE_METHOD_CALL(init_call);
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
        [[nodiscard]] const Expr *build_string_expr(StringSpan value, SourceLocation loc);
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
            StringSpan id;
            std::vector<Parameter> parameter_list;

            void reset() { id.clear(), parameter_list.clear(); }
        } function_draft_;

        u32 next_function_address_ = k_first_function_address;

        explicit Restricted(const SemanticSystemServices &ss_services);
        ~Restricted() override = default;

        void update_function_draft();
        void update_function_draft(StringSpan id);
        void collect_function_parameter(StringSpan id, SourceLocation id_loc);
        [[nodiscard]] const Expr *forward_program_function(
            const ProgFuncSymbol *func_symbol, SourceLocation result_loc);
        [[nodiscard]] const ProgFuncSymbol *build_program_function_entry(
            SourceLocation func_signature_loc);
        [[nodiscard]] const ProgFuncSymbol *build_program_function_exit(
            BlockSourceLocation block_loc);

        void register_function_parameters();
        [[nodiscard]] bool validate_funcdef_name(StringSpan func_name, SourceLocation funcname_loc);
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
            StringSpan member_id,
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
AggregateBuilder::Restricted::begin_dict_entry()
{
    parse_ctx_->table_ctx_handler.enter_dict_entry();
}

inline void
AggregateBuilder::Restricted::end_dict_entry() { parse_ctx_->table_ctx_handler.exit_dict_entry(); }
} // namespace alpha
#endif // EXPR_BUILDERS_HPP
