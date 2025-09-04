#include "L2_semantic_subsystems/control_flow_managers.hpp"

namespace alpha
{
ControlFlowManager::ControlFlowManager(const SemanticSystemServices &ss_services)
    :DISPATCH_TARGET(ss_services) {}

ControlFlowManager::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

void
ControlFlowManager::Restricted::manage_ifbranch_entry(
    const Expr *const conditional,
    const SourceLocation if_clause_loc)
{
    DEBUG_SMART_ASSERT(!!conditional);
    const Expr *const materialized_conditional = ss_bridge_->materialize_if_table_item(conditional);
    ss_bridge_->finalize_bool_expr(materialized_conditional);

    auto *const qh = quad_handler_; // Short alias for readability.

    // Offset = 2 the IF_EQ itself and the following unconditional jump which leads outside if block.
    constexpr LabelID offset_to_if_branch = 2;
    qh->emit_next(
        ir::Opcode::IF_EQ,
        nullptr,
        materialized_conditional,
        &k_static_true_expr,
        if_clause_loc,
        offset_to_if_branch
    );
    // Record the placeholder label for the 'false' branch jump to be patched later.
    build_ctx_.unpatched_if_bypass_jumps.push(qh->next_quad_label());
    // Emit unconditional jump that will eventually point at the end of the if-block.
    qh->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, if_clause_loc);
}

void
ControlFlowManager::Restricted::manage_ifbranch_exit()
{
    DEBUG_SMART_ASSERT(!build_ctx_.unpatched_if_bypass_jumps.empty());

    auto *const qh = quad_handler_; // Short aliases for readability.

    const LabelID bypass_jump_quad_label = build_ctx_.unpatched_if_bypass_jumps.top();
    build_ctx_.unpatched_if_bypass_jumps.pop();
    qh->patch_quad(bypass_jump_quad_label, qh->next_quad_label());
}

void
ControlFlowManager::Restricted::manage_elsebranch_entry(const SourceLocation else_clause_loc)
{
    DEBUG_SMART_ASSERT(
        !build_ctx_.unpatched_if_bypass_jumps.empty() &&
        "For an 'else' statement to exist, there must be a preceding 'if'"
    );

    auto *const qh = quad_handler_; // Short alias for readability.
    build_ctx_.unpatched_else_bypass_jumps.push(qh->next_quad_label());
    qh->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, else_clause_loc);
}

void
ControlFlowManager::Restricted::manage_elsebranch_exit()
{
    DEBUG_SMART_ASSERT(
        !build_ctx_.unpatched_else_bypass_jumps.empty()
       ,
        !build_ctx_.unpatched_if_bypass_jumps.empty() &&
        "For an 'else' statement to exist, there must be a preceding 'if'"
    );

    auto *const qh = quad_handler_; // Short alias for readability.

    // We basically patch untaken if branches inside else branch.
    qh->patch_quad(
        build_ctx_.unpatched_if_bypass_jumps.top(),
        build_ctx_.unpatched_else_bypass_jumps.top() + 1
    );
    build_ctx_.unpatched_if_bypass_jumps.pop();
    qh->patch_quad(build_ctx_.unpatched_else_bypass_jumps.top(), qh->next_quad_label());

    build_ctx_.unpatched_else_bypass_jumps.pop();
}

void
ControlFlowManager::Restricted::manage_whileloop_entry()
{
    build_ctx_.push_new_whileloop_frame();
    DEBUG(
        const auto &wlf_stack = build_ctx_.while_loop_frames;
        SMART_ASSERT(!wlf_stack.empty());
    )
    build_ctx_.while_loop_frames.top().before_condition = quad_handler_->next_quad_label();
}

void
ControlFlowManager::Restricted::manage_whileloop_condition(
    const Expr *const conditional,
    const SourceLocation while_clause_loc)
{
    DEBUG_SMART_ASSERT(!!conditional);

    const Expr *const materialized_conditional = ss_bridge_->materialize_if_table_item(conditional);
    ss_bridge_->finalize_bool_expr(materialized_conditional);

    auto *const qh = quad_handler_; // Short alias for readability.

    constexpr LabelID offset_to_while_block = 2;
    qh->emit_next(
        ir::Opcode::IF_EQ,
        nullptr,
        materialized_conditional,
        &k_static_true_expr,
        while_clause_loc,
        offset_to_while_block
    );
    // Record the placeholder label for the 'false' condition jump to be patched later.
    build_ctx_.while_loop_frames.top().unpatched_bypass_jump = qh->next_quad_label();
    // Emit unconditional jump that will eventually point at the end of the while-block.
    qh->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, while_clause_loc);

    parse_ctx_->func_ctx_handler.enter_loop();
}

void
ControlFlowManager::Restricted::manage_whileloop_exit(const SourceLocation while_stmt_loc)
{
    DEBUG(
        auto &wlf_stack = build_ctx_.while_loop_frames;
        SMART_ASSERT(!wlf_stack.empty());
        SMART_ASSERT(
            wlf_stack.top().before_condition != k_no_label,
            wlf_stack.top().unpatched_bypass_jump != k_no_label,
        );
    )
    auto *const qh = quad_handler_; // Short alias for readability.
    auto &wlf = build_ctx_.while_loop_frames.top();
    auto &fctx = parse_ctx_->func_ctx_handler; // Short alias for readability.

    qh->emit(
        ir::Opcode::JUMP,
        nullptr,
        nullptr,
        nullptr,
        while_stmt_loc,
        wlf.before_condition
    );

    qh->patch_quad(wlf.unpatched_bypass_jump, qh->next_quad_label());

    qh->patch_list(fctx.break_list(), qh->next_quad_label());
    qh->patch_list(fctx.continue_list(), wlf.before_condition);

    parse_ctx_->func_ctx_handler.exit_loop(); // Kills break and continue lists.
    build_ctx_.while_loop_frames.pop();       // DO NOT USE `wlf` PAST THIS POINT
}

void
ControlFlowManager::Restricted::mark_forloop_condition_entry()
{
    build_ctx_.push_new_forloop_frame();
    DEBUG(
        auto &flf_stack = build_ctx_.for_loop_frames;
        SMART_ASSERT(!flf_stack.empty());
        SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::BEFORE_CONDITION);
    )
    mark_upcoming_forloop_sites();
}

void
ControlFlowManager::Restricted::mark_forloop_update_list_entry()
{
    DEBUG(
        auto &flf_stack = build_ctx_.for_loop_frames;
        SMART_ASSERT(!flf_stack.empty());
        SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::BEFORE_UPDATE_LIST);
    )
    mark_upcoming_forloop_sites();
}

void
ControlFlowManager::Restricted::mark_forloop_update_list_exit(const SourceLocation exit_loc)
{
    DEBUG(
        auto &flf_stack = build_ctx_.for_loop_frames;
        SMART_ASSERT(!flf_stack.empty());
        SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::AFTER_UPDATE_LIST);
    )
    mark_upcoming_forloop_sites();
    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, exit_loc);
}

void
ControlFlowManager::Restricted::manage_forloop_entry()
{
    DEBUG(
        auto & flf_stack = build_ctx_.for_loop_frames;
        SMART_ASSERT(!flf_stack.empty());
        SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::BEFORE_BODY);
    )
    mark_upcoming_forloop_sites();
    parse_ctx_->func_ctx_handler.enter_loop();
}

void
ControlFlowManager::Restricted::manage_forloop_condition(
    const Expr *const conditional,
    const SourceLocation condition_loc)
{
    DEBUG(
        auto &flf_stack = build_ctx_.for_loop_frames;
        SMART_ASSERT(!flf_stack.empty());
    )
    DEBUG_SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::CONDITION_TRUE);

    const Expr *const materialized_conditional = ss_bridge_->materialize_if_table_item(conditional);
    ss_bridge_->finalize_bool_expr(materialized_conditional);

    auto *const qh = quad_handler_;
    mark_upcoming_forloop_sites();
    qh->emit_labelless(
        ir::Opcode::IF_EQ,
        nullptr,
        materialized_conditional,
        &k_static_true_expr,
        condition_loc
    );

    DEBUG_SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::CONDITION_FALSE);
    mark_upcoming_forloop_sites();
    qh->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, condition_loc);
}

void
ControlFlowManager::Restricted::manage_forloop_exit(const SourceLocation exit_loc)
{
    DEBUG(
        auto & flf_stack = build_ctx_.for_loop_frames;
        SMART_ASSERT(!flf_stack.empty());
    )

    auto *const qh = quad_handler_;               // Short alias to improve readability.
    auto &flf = build_ctx_.for_loop_frames.top(); // Short alias to improve readability.

    // Emit closure loop jump.
    DEBUG_SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::AFTER_BODY);
    mark_upcoming_forloop_sites();
    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, exit_loc);

    const LabelID after_loop_quad_label = qh->next_quad_label(); // First quad outside for-loop.

    qh->patch_quad(flf.condition_true, flf.before_body); // Set IF_EQ true jump inside body
    qh->patch_quad(flf.condition_false, after_loop_quad_label); // Set IF_EQ false jump outside body
    qh->patch_quad(flf.after_update_list, flf.before_condition); // After update go check condition
    qh->patch_quad(flf.after_body, flf.before_update_list); // After closure go update iterators

    // We route all breaks outside the body of the for loop.
    qh->patch_list(parse_ctx_->func_ctx_handler.break_list(), after_loop_quad_label);
    // We route all continues at the beginning of the update_list
    qh->patch_list(parse_ctx_->func_ctx_handler.continue_list(), flf.before_update_list);

    parse_ctx_->func_ctx_handler.exit_loop(); // This kills break and continue lists.
    build_ctx_.for_loop_frames.pop();         // DO NOT USE `flf` PAST THIS POINT
}

void
ControlFlowManager::Restricted::manage_break(const SourceLocation break_loc)
{
    manage_loop_keyword(LoopKeyword::BREAK, break_loc);
}

void
ControlFlowManager::Restricted::manage_continue(const SourceLocation continue_loc)
{
    manage_loop_keyword(LoopKeyword::CONTINUE, continue_loc);
}

void
ControlFlowManager::Restricted::manage_return(
    const SourceLocation return_loc,
    const Expr *const retval)
{
    if (parse_ctx_->func_ctx_handler.function_nesting_depth() == 0)
    {
        dr_->report_return_keyword_outside_func(return_loc);
        return;
    }

    const Expr *const materialized_retval = ss_bridge_->materialize_if_table_item(retval);
    ss_bridge_->finalize_bool_expr(materialized_retval);

    quad_handler_->emit_next(ir::Opcode::RETURN, nullptr, materialized_retval, nullptr, return_loc);
    parse_ctx_->func_ctx_handler.add_label_to_returnlist(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, return_loc);
}

void
ControlFlowManager::Restricted::mark_upcoming_forloop_sites()
{
    DEBUG_SMART_ASSERT(!build_ctx_.for_loop_frames.empty());

    using FLPP = ForLoopSite;

    auto &flf = build_ctx_.for_loop_frames.top();

    // clang-format off
    switch (const LabelID next_jump_label = quad_handler_->next_quad_label(); flf.next_patch_point)
    {
    case FLPP::BEFORE_CONDITION:   flf.before_condition = next_jump_label;   break;
    case FLPP::CONDITION_TRUE:     flf.condition_true = next_jump_label;     break;
    case FLPP::CONDITION_FALSE:    flf.condition_false = next_jump_label;    break;
    case FLPP::BEFORE_UPDATE_LIST: flf.before_update_list = next_jump_label; break;
    case FLPP::AFTER_UPDATE_LIST:  flf.after_update_list = next_jump_label;  break;
    case FLPP::BEFORE_BODY:        flf.before_body = next_jump_label;        break;
    case FLPP::AFTER_BODY:         flf.after_body = next_jump_label;         break;
    default: [[unlikely]] UNREACHABLE(FMT::format(
        "Unknown patch_point: int(patch_point) = {}", static_cast<int>(flf.next_patch_point)));
    }
    // clang-format on

    using UT = std::underlying_type_t<FLPP>;
    if (flf.next_patch_point != FLPP::AFTER_BODY)
        flf.next_patch_point = static_cast<FLPP>(static_cast<UT>(flf.next_patch_point) + 1);
}

bool
ControlFlowManager::Restricted::is_in_loop()
{
    return parse_ctx_->func_ctx_handler.loop_depth() > 0;
}

void
ControlFlowManager::Restricted::manage_loop_keyword(
    const LoopKeyword keyword,
    const SourceLocation keyword_loc)
{
    if (!is_in_loop())
    {
        dr_->report_loop_ctrl_keyword_outside_loop(keyword, keyword_loc);
        return;
    }
    switch (keyword)
    {
    case LoopKeyword::BREAK:
        parse_ctx_->func_ctx_handler.add_label_to_breaklist(quad_handler_->next_quad_label());
        break;
    case LoopKeyword::CONTINUE:
        parse_ctx_->func_ctx_handler.add_label_to_continuelist(quad_handler_->next_quad_label());
        break;
    default: [[unlikely]] UNREACHABLE(FMT::format(
            "Unknown keyword: int(keyword) = {}", static_cast<int>(keyword)));
    }

    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, keyword_loc);
}

ControlFlowManager::ForLoopSite
ControlFlowManager::Restricted::next(const ForLoopSite fls) noexcept
{
    using FLS = ForLoopSite;
    switch (fls)
    {
    case FLS::BEFORE_CONDITION: return FLS::CONDITION_TRUE;
    case FLS::CONDITION_TRUE: return FLS::CONDITION_FALSE;
    case FLS::CONDITION_FALSE: return FLS::BEFORE_UPDATE_LIST;
    case FLS::BEFORE_UPDATE_LIST: return FLS::AFTER_UPDATE_LIST;
    case FLS::AFTER_UPDATE_LIST: return FLS::BEFORE_BODY;
    case FLS::BEFORE_BODY: return FLS::AFTER_BODY;
    case FLS::AFTER_BODY: return FLS::BEFORE_CONDITION;
    default: [[unlikely]] UNREACHABLE("Unknown FLS");
    }
}
} // namespace alpha
