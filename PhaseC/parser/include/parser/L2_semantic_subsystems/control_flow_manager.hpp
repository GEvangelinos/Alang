#ifndef CONTROL_FLOW_MANAGER_HPP
#define CONTROL_FLOW_MANAGER_HPP

#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include "parser_context.hpp"
#include "core/source_location.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include "core/quad_emitter.hpp"

#include  <parser/ir_opcode.gen.hpp>
#include "semantic_subsystem.hpp"

namespace alpha
{
class ControlFlowManager
{
    friend class SemanticSystem;

private:
    enum class LoopKeyword { CONTINUE, BREAK };

    enum class ForLoopSite : u8
    {
        BEFORE_CONDITION,
        CONDITION_TRUE,
        CONDITION_FALSE,
        BEFORE_UPDATE_LIST,
        AFTER_UPDATE_LIST,
        BEFORE_BODY,
        AFTER_BODY,
    };

    // Friending outer namespace so its visible by ADL, making it usable by DiagnosticReporter.
    [[nodiscard]] friend const char *to_string(const LoopKeyword lk)
    {
        switch (lk)
        {
        case LoopKeyword::BREAK: return "break";
        case LoopKeyword::CONTINUE: return "continue";
        default: [[unlikely]] UNREACHABLE(FMT::format(
                "Unknown LoopKeyword: int(lk) = {}", static_cast<int>(lk)));
        }
    }

    class Restricted final : private SemanticSubsystem
    {
        friend class ControlFlowManager;

    private:
        struct // (singleton)
        {
            using LabelStack = VectorStack<LabelID>;
            LabelStack unpatched_if_bypass_jumps;
            LabelStack unpatched_else_bypass_jumps;

            struct WhileLoopPatchPoints
            {
                LabelID unpatched_bypass_jump = k_no_label;
                LabelID before_condition = k_no_label;
            };

            struct ForLoopPatchPoints
            {
                ForLoopSite next_patch_point = ForLoopSite::BEFORE_CONDITION;
                LabelID before_condition = k_no_label;
                LabelID condition_true = k_no_label;
                LabelID condition_false = k_no_label;
                LabelID before_update_list = k_no_label;
                LabelID after_update_list = k_no_label;
                LabelID before_body = k_no_label;
                LabelID after_body = k_no_label;
                bool bad_clause;
            };

            VectorStack<WhileLoopPatchPoints> while_loop_patch_points;
            VectorStack<ForLoopPatchPoints> for_loop_patch_points;

            void push_new_whileloop_patch_point_frame() { while_loop_patch_points.emplace(); }
            void push_new_forloop_patch_point_frame() { for_loop_patch_points.emplace(); }
        } build_ctx_;

        explicit Restricted(const SemanticSystemServices &ss_services);

        void manage_ifbranch_entry(const Expr *conditional, SourceLocation if_clause_loc);
        void manage_ifbranch_exit();
        void manage_elsebranch_entry(SourceLocation else_clause_loc);
        void manage_elsebranch_exit();
        void manage_whileloop_entry();
        void manage_whileloop_condition(const Expr *conditional, SourceLocation while_clause_loc);
        void manage_whileloop_exit(SourceLocation while_stmt_loc);
        void mark_forloop_condition_entry();
        void mark_forloop_update_list_entry();
        void mark_forloop_update_list_exit(SourceLocation exit_loc);
        void manage_forloop_condition(const Expr *conditional, SourceLocation condition_loc);
        void manage_forloop_entry();
        void manage_forloop_exit(SourceLocation exit_loc);
        void enter_forloop_clause();
        void exit_forloop_clause();
        void mark_bad_forloop_clause();
        void manage_break(SourceLocation break_loc);
        void manage_continue(SourceLocation continue_loc);
        void manage_return(SourceLocation return_loc, const Expr *retval = nullptr);

        void mark_upcoming_forloop_sites();
        void manage_loop_keyword(LoopKeyword keyword, SourceLocation keyword_loc);
        bool is_in_loop();

        static ForLoopSite next(ForLoopSite fls) noexcept;
    };

    // Accessors exists to insulate call sites from the DISPATCH_TARGET macro
    // and to make the intended access point to Restricted state explicit.
    Restricted DISPATCH_TARGET;
    Restricted &restricted() noexcept { return DISPATCH_TARGET; }
    const Restricted &restricted() const noexcept { return DISPATCH_TARGET; }

    explicit ControlFlowManager(const SemanticSystemServices &ss_services);

    // Defined outside Restricted, so it can be accessed by SemanticSystem's generalized expr collector
    void commit_forloop_header_expr(const Expr *header_expr);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(manage_ifbranch_entry);
    DISPATCH_SLAVE_METHOD_CALL(manage_ifbranch_exit);
    DISPATCH_SLAVE_METHOD_CALL(manage_elsebranch_entry);
    DISPATCH_SLAVE_METHOD_CALL(manage_elsebranch_exit);
    DISPATCH_SLAVE_METHOD_CALL(manage_whileloop_entry);
    DISPATCH_SLAVE_METHOD_CALL(manage_whileloop_condition);
    DISPATCH_SLAVE_METHOD_CALL(manage_whileloop_exit);
    DISPATCH_SLAVE_METHOD_CALL(manage_break);
    DISPATCH_SLAVE_METHOD_CALL(manage_continue);
    DISPATCH_SLAVE_METHOD_CALL(manage_return);
    DISPATCH_SLAVE_METHOD_CALL(mark_forloop_condition_entry);
    DISPATCH_SLAVE_METHOD_CALL(mark_forloop_update_list_entry);
    DISPATCH_SLAVE_METHOD_CALL(mark_forloop_update_list_exit);
    DISPATCH_SLAVE_METHOD_CALL(manage_forloop_condition);
    DISPATCH_SLAVE_METHOD_CALL(manage_forloop_entry);
    DISPATCH_SLAVE_METHOD_CALL(manage_forloop_exit);
    DISPATCH_SLAVE_METHOD_CALL(mark_bad_forloop_clause);
    DISPATCH_SLAVE_METHOD_CALL(enter_forloop_clause);
    DISPATCH_SLAVE_METHOD_CALL(exit_forloop_clause);
    DISPATCH_DEFINE_HANDLER_END();
};
} // namespace alpha
#endif // CONTROL_FLOW_MANAGER_HPP
