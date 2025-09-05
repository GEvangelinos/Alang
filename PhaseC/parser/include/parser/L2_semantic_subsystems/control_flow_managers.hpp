#ifndef CONTROL_FLOW_HANDLERS_HPP
#define CONTROL_FLOW_HANDLERS_HPP

#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include "parser_context.hpp"
#include "core/source_location.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include "L3_ir_infra/quad_handler.hpp"

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
            using LabelStack = std::stack<LabelID, std::vector<LabelID>>;
            LabelStack unpatched_if_bypass_jumps;
            LabelStack unpatched_else_bypass_jumps;

            struct WhileLoopPatchPoints
            {
                LabelID unpatched_bypass_jump;
                LabelID before_condition;
            };

            struct ForLoopPatchPoints
            {
                ForLoopSite next_patch_point = ForLoopSite::BEFORE_CONDITION;
                LabelID before_condition;
                LabelID condition_true;
                LabelID condition_false;
                LabelID before_update_list;
                LabelID after_update_list;
                LabelID before_body;
                LabelID after_body;
            };

            std::stack<WhileLoopPatchPoints, std::vector<WhileLoopPatchPoints>> while_loop_frames;
            std::stack<ForLoopPatchPoints, std::vector<ForLoopPatchPoints>> for_loop_frames;

            void push_new_whileloop_frame() { while_loop_frames.push(WhileLoopPatchPoints()); }
            void push_new_forloop_frame() { for_loop_frames.push(ForLoopPatchPoints()); }
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
        void manage_break(SourceLocation break_loc);
        void manage_continue(SourceLocation continue_loc);
        void manage_return(SourceLocation return_loc, const Expr *retval = nullptr);

        void mark_upcoming_forloop_sites();
        void manage_loop_keyword(LoopKeyword keyword, SourceLocation keyword_loc);
        bool is_in_loop();

        static ForLoopSite next(ForLoopSite fls) noexcept;
    };

    Restricted DISPATCH_TARGET;

    explicit ControlFlowManager(const SemanticSystemServices &ss_services);

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
    DISPATCH_SLAVE_METHOD_CALL(enter_forloop_clause);
    DISPATCH_SLAVE_METHOD_CALL(exit_forloop_clause);
    DISPATCH_DEFINE_HANDLER_END();
};

} // namespace alpha
#endif // CONTROL_FLOW_HANDLERS_HPP
