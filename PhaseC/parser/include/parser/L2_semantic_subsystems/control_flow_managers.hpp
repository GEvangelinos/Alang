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

    // to_string() function is declared outside class ControlFlowManager so its visible by ADL, to be used by DiagnosticReporter.
    friend const char *to_string(const LoopKeyword lk)
    {
        switch (lk)
        {
        case LoopKeyword::BREAK: return "break";
        case LoopKeyword::CONTINUE: return "continue";
        default:
            UNREACHABLE(FMT::format("Unknown LoopKeyword: int(lk) = {}", static_cast<int>(lk)));
        }
    }

    class Restricted final : private SemanticSubsystem
    {
        friend class ControlFlowManager;

    private:
        // Control-Flow-State
        struct
        {
            using LabelStack = std::stack<LabelID, std::vector<LabelID>>;
            LabelStack unpatched_if_bypass_jumps;
            LabelStack unpatched_else_bypass_jumps;
            LabelStack unpatched_while_bypass_jumps;
            LabelStack while_condition_head_labels;
            LabelStack for_condition_head_labels;
        } ctrl_flow_state_;

        explicit Restricted(const SemanticSystemServices &ss_services);

        void manage_if_entry(const Expr *conditional, SourceLocation if_clause_loc);
        void manage_if_exit();
        void manage_else_entry(SourceLocation else_clause_loc);
        void manage_else_exit();
        void manage_while_entry();
        void manage_while_condition(const Expr *conditional, SourceLocation while_clause_loc);
        void manage_while_exit(SourceLocation while_stmt_loc);
        void manage_break(SourceLocation break_loc);
        void manage_continue(SourceLocation continue_loc);

        bool is_in_loop();

        template<LoopKeyword keyword>
        void process_loop_keyword(SourceLocation keyword_loc);
    };

    Restricted DISPATCH_TARGET;

    explicit ControlFlowManager(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(manage_if_entry);
    DISPATCH_SLAVE_METHOD_CALL(manage_if_exit);
    DISPATCH_SLAVE_METHOD_CALL(manage_else_entry);
    DISPATCH_SLAVE_METHOD_CALL(manage_else_exit);
    DISPATCH_SLAVE_METHOD_CALL(manage_while_entry);
    DISPATCH_SLAVE_METHOD_CALL(manage_while_condition);
    DISPATCH_SLAVE_METHOD_CALL(manage_while_exit);
    DISPATCH_SLAVE_METHOD_CALL(manage_break);
    DISPATCH_SLAVE_METHOD_CALL(manage_continue);
    DISPATCH_DEFINE_HANDLER_END();
};

inline void
ControlFlowManager::Restricted::manage_if_entry(
    const Expr *const conditional,
    const SourceLocation if_clause_loc)
{
    DEBUG_SMART_ASSERT(!!conditional, SemUtils::is_bool_or_const_bool_expr(conditional));

    auto *const qh = quad_handler_; // Short alias for readability.

    // Offset = 2 the IF_EQ itself and the following unconditional jump which leads outside if block.
    constexpr LabelID offset_to_if_branch = 2;
    qh->emit_next(
        ir::Opcode::IF_EQ,
        nullptr,
        conditional,
        &k_static_true_expr,
        if_clause_loc,
        offset_to_if_branch
    );
    // Record the placeholder label for the 'false' branch jump to be patched later.
    ctrl_flow_state_.unpatched_if_bypass_jumps.push(qh->next_quad_label());
    // Emit unconditional jump that will eventually target the end of the if-block.
    qh->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, if_clause_loc);
}

inline void
ControlFlowManager::Restricted::manage_if_exit()
{
    DEBUG_SMART_ASSERT(!ctrl_flow_state_.unpatched_if_bypass_jumps.empty());

    auto *const qh = quad_handler_; // Short aliases for readability.

    const LabelID bypass_jump_quad_label = ctrl_flow_state_.unpatched_if_bypass_jumps.top();
    ctrl_flow_state_.unpatched_if_bypass_jumps.pop();
    qh->patch_quad(bypass_jump_quad_label, qh->next_quad_label());
}

inline void
ControlFlowManager::Restricted::manage_else_entry(const SourceLocation else_clause_loc)
{
    DEBUG_SMART_ASSERT(
        !ctrl_flow_state_.unpatched_if_bypass_jumps.empty() &&
        "For an 'else' statement to exist, there must be a preceding 'if'"
        // This holds true with the current grammar, since an 'else' can only be
        // captured if it is preceded by an 'if' statement. If, in the future, we
        // split the grammar for more refined diagnostics, this assertion should be
        // removed, as the parser would then handle such cases gracefully. At that
        // point, encountering an 'else' without a preceding 'if' would no longer
        // represent a logic error in the semantic-system of the parser.
    );

    auto *const qh = quad_handler_; // Short alias for readability.
    ctrl_flow_state_.unpatched_else_bypass_jumps.push(qh->next_quad_label());
    qh->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, else_clause_loc);
}

inline void
ControlFlowManager::Restricted::manage_else_exit()
{
    DEBUG_SMART_ASSERT(
        !ctrl_flow_state_.unpatched_else_bypass_jumps.empty()
       ,
        !ctrl_flow_state_.unpatched_if_bypass_jumps.empty() &&
        "For an 'else' statement to exist, there must be a preceding 'if'"
        // This holds true with the current grammar, since an 'else' can only be
        // captured if it is preceded by an 'if' statement. If, in the future, we
        // split the grammar for more refined diagnostics, this assertion should be
        // removed, as the parser would then handle such cases gracefully. At that
        // point, encountering an 'else' without a preceding 'if' would no longer
        // represent a logic error in the semantic-system of the parser.
    );

    auto *const qh = quad_handler_; // Short alias for readability.

    // We basically patch untaken if branches inside else branch.
    qh->patch_quad(
        ctrl_flow_state_.unpatched_if_bypass_jumps.top(),
        ctrl_flow_state_.unpatched_else_bypass_jumps.top()
    );

    const LabelID bypass_jump_quad_label = ctrl_flow_state_.unpatched_else_bypass_jumps.top();
    ctrl_flow_state_.unpatched_else_bypass_jumps.pop();
    qh->patch_quad(bypass_jump_quad_label, qh->next_quad_label());
}

inline void
ControlFlowManager::Restricted::manage_while_entry()
{
    ctrl_flow_state_.while_condition_head_labels.push(quad_handler_->next_quad_label());
}

inline void
ControlFlowManager::Restricted::manage_while_condition(
    const Expr *const conditional,
    const SourceLocation while_clause_loc)
{
    DEBUG_SMART_ASSERT(!!conditional, SemUtils::is_bool_or_const_bool_expr(conditional));
    auto *const qh = quad_handler_; // Short alias for readability.

    constexpr LabelID offset_to_while_block = 2;
    qh->emit_next(
        ir::Opcode::IF_EQ,
        nullptr,
        conditional,
        &k_static_true_expr,
        while_clause_loc,
        offset_to_while_block
    );
    // Record the placeholder label for the 'false' condition jump to be patched later.
    ctrl_flow_state_.unpatched_while_bypass_jumps.push(qh->next_quad_label());
    // Emit unconditional jump that will eventually target the end of the while-block.
    qh->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, while_clause_loc);

    parse_ctx_->func_ctx_handler.enter_loop();
}

inline void
ControlFlowManager::Restricted::manage_while_exit(const SourceLocation while_stmt_loc)
{
    DEBUG_SMART_ASSERT(
        !ctrl_flow_state_.while_condition_head_labels.empty(),
        !ctrl_flow_state_.unpatched_while_bypass_jumps.empty()
    );
    auto *const qh = quad_handler_;            // Short alias for readability.
    auto &fctx = parse_ctx_->func_ctx_handler; // Short alias for readability.

    qh->emit(
        ir::Opcode::JUMP,
        nullptr,
        nullptr,
        nullptr,
        while_stmt_loc,
        ctrl_flow_state_.while_condition_head_labels.top()
    );

    qh->patch_quad(ctrl_flow_state_.unpatched_while_bypass_jumps.top(), qh->next_quad_label());
    ctrl_flow_state_.unpatched_while_bypass_jumps.pop();

    qh->patch_list(fctx.break_list(), qh->next_quad_label());
    qh->patch_list(fctx.continue_list(), ctrl_flow_state_.while_condition_head_labels.top());
    ctrl_flow_state_.while_condition_head_labels.pop();

    parse_ctx_->func_ctx_handler.exit_loop(); // Kills break and continue lists.
}

inline void
ControlFlowManager::Restricted::manage_break(const SourceLocation break_loc)
{
    process_loop_keyword<LoopKeyword::BREAK>(break_loc);
}

inline void
ControlFlowManager::Restricted::manage_continue(const SourceLocation continue_loc)
{
    process_loop_keyword<LoopKeyword::CONTINUE>(continue_loc);
}

inline bool
ControlFlowManager::Restricted::is_in_loop()
{
    return parse_ctx_->func_ctx_handler.loop_depth() > 0;
}

template<ControlFlowManager::LoopKeyword keyword>
void ControlFlowManager::Restricted::process_loop_keyword(const SourceLocation keyword_loc)
{
    if (!is_in_loop())
    {
        dr_->report_loop_ctrl_keyword_outside_loop(keyword, keyword_loc);
        return;
    }
    if constexpr (keyword == LoopKeyword::BREAK)
        parse_ctx_->func_ctx_handler.add_label_to_breaklist(quad_handler_->next_quad_label());
    else if constexpr (keyword == LoopKeyword::CONTINUE)
        parse_ctx_->func_ctx_handler.add_label_to_continuelist(quad_handler_->next_quad_label());
    else
        static_assert([]() { return false; }(), "Unknown keyword");

    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, keyword_loc);
}
} // namespace alpha
#endif // CONTROL_FLOW_HANDLERS_HPP
