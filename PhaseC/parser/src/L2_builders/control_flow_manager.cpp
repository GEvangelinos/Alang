#include "L2_semantic_subsystems/control_flow_manager.hpp"

#include "L2_semantic_subsystems/core/expr_normalizer.hpp"

namespace alpha
{
ControlFlowManager::ControlFlowManager(
    const SemanticSystemServices& ss_services)
    : DISPATCH_TARGET(ss_services) {}

ControlFlowManager::Restricted::Restricted(const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services) {}

void
ControlFlowManager::Restricted::manage_ifbranch_entry(
    const Expr* conditional,
    const SourceLocation if_clause_loc)
{
    DMASSERT(!!conditional);
    conditional = expr_optimizer_->try_propagate_const(conditional);
    conditional = expr_normalizer_->materialize_if_table_item(conditional);
    expr_normalizer_->resolve_bool_short_circuit(conditional);

    if (conditional->is_static())
    {
        if (SemUtils::as_bool(conditional))
            parse_ctx_->func_ctx_handler.flow_liveness().push_if_flow_state(
                FunctionCtxHandler::FlowLivenessTracker::FlowState::ALIVE);
        else
            parse_ctx_->func_ctx_handler.flow_liveness().push_if_flow_state(
                FunctionCtxHandler::FlowLivenessTracker::FlowState::DEAD);
    }
    else
        parse_ctx_->func_ctx_handler.flow_liveness().push_if_flow_state(
            FunctionCtxHandler::FlowLivenessTracker::FlowState::RUNTIME);

    // Offset = 2 the IF_EQ itself and the following unconditional jump which leads outside if block.
    constexpr CodeAddress offset_to_if_branch = CodeAddress{2};
    quad_yielder_->yield_next(
        ir::Opcode::IF_EQ,
        nullptr,
        conditional,
        &k_static_true_expr,
        if_clause_loc,
        offset_to_if_branch
    );
    // Record the placeholder label for the 'false' branch jump to be patched later.
    build_ctx_.unpatched_if_bypass_jumps.push(quad_handler_->next_quad_label());
    // Emit unconditional jump that will eventually point at the end of the if-block.
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, if_clause_loc);
}

void
ControlFlowManager::Restricted::manage_ifbranch_exit()
{
    DMASSERT(!build_ctx_.unpatched_if_bypass_jumps.empty());
    const CodeAddress bypass_jump_quad_label = build_ctx_.unpatched_if_bypass_jumps.top();
    build_ctx_.unpatched_if_bypass_jumps.pop();
    quad_handler_->labelPatch_quad(bypass_jump_quad_label, quad_handler_->next_quad_label());
    parse_ctx_->func_ctx_handler.flow_liveness().pop_flow_state();
}

void
ControlFlowManager::Restricted::manage_elsebranch_entry(const SourceLocation else_clause_loc)
{
    DMASSERT(
        !build_ctx_.unpatched_if_bypass_jumps.empty() &&
        "For an 'else' statement to exist, there must be a preceding 'if'"
    );
    build_ctx_.unpatched_else_bypass_jumps.push(quad_handler_->next_quad_label());
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, else_clause_loc);
    parse_ctx_->func_ctx_handler.flow_liveness().switch_to_else();
}

void
ControlFlowManager::Restricted::manage_elsebranch_exit()
{
    DMASSERT(
        !build_ctx_.unpatched_else_bypass_jumps.empty()
        ,
        !build_ctx_.unpatched_if_bypass_jumps.empty() &&
        "For an 'else' statement to exist, there must be a preceding 'if'"
    );

    // We basically patch untaken if branches inside else branch.
    quad_handler_->labelPatch_quad(
        build_ctx_.unpatched_if_bypass_jumps.top(),
        build_ctx_.unpatched_else_bypass_jumps.top() + CodeAddress{1}
    );
    quad_handler_->labelPatch_quad(
        build_ctx_.unpatched_else_bypass_jumps.top(),
        quad_handler_->next_quad_label()
    );

    build_ctx_.unpatched_if_bypass_jumps.pop();
    build_ctx_.unpatched_else_bypass_jumps.pop();
    parse_ctx_->func_ctx_handler.flow_liveness().pop_flow_state();
}

void
ControlFlowManager::Restricted::manage_whileloop_entry()
{
    build_ctx_.push_new_whileloop_patch_point_frame();
    DEBUG(
        const auto &wlf_stack = build_ctx_.while_loop_patch_points;
        SMART_ASSERT(!wlf_stack.empty());
    )
    build_ctx_.while_loop_patch_points.top().before_condition = quad_handler_->next_quad_label();
}

void
ControlFlowManager::Restricted::manage_whileloop_condition(
    const Expr* conditional,
    const SourceLocation while_clause_loc)
{
    DMASSERT(!!conditional);

    constexpr CodeAddress offset_to_while_block{2};
    conditional = expr_optimizer_->try_propagate_const(conditional);
    conditional = expr_normalizer_->materialize_if_table_item(conditional);
    expr_normalizer_->resolve_bool_short_circuit(conditional);
    quad_yielder_->yield_next(
        ir::Opcode::IF_EQ,
        nullptr,
        conditional,
        &k_static_true_expr,
        while_clause_loc,
        offset_to_while_block
    );
    // Record the placeholder label for the 'false' condition jump to be patched later.
    build_ctx_.while_loop_patch_points.top().unpatched_bypass_jump =
        quad_handler_->next_quad_label();
    // Emit unconditional jump that will eventually point at the end of the while-block.
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, while_clause_loc);

    parse_ctx_->func_ctx_handler.enter_loop();
}

void
ControlFlowManager::Restricted::manage_whileloop_exit(const SourceLocation while_stmt_loc)
{
    DEBUG(
        auto &wlf_stack = build_ctx_.while_loop_patch_points;
        SMART_ASSERT(!wlf_stack.empty());
        SMART_ASSERT(
            wlf_stack.top().before_condition != CodeAddress::none(),
            wlf_stack.top().unpatched_bypass_jump != CodeAddress::none(),
        );
    )
    auto& qe = quad_handler_;                                   // Short alias for readability.
    const auto& wlf = build_ctx_.while_loop_patch_points.top(); // Short alias for readability.
    auto& fctx = parse_ctx_->func_ctx_handler;                  // Short alias for readability.

    quad_yielder_->yield(
        ir::Opcode::JUMP,
        nullptr,
        nullptr,
        nullptr,
        while_stmt_loc,
        wlf.before_condition
    );

    qe->labelPatch_quad(wlf.unpatched_bypass_jump, qe->next_quad_label());

    qe->labelPatch_list(fctx.break_list(), qe->next_quad_label());
    qe->labelPatch_list(fctx.continue_list(), wlf.before_condition);

    parse_ctx_->func_ctx_handler.exit_loop(); // Kills break and continue lists.
    build_ctx_.while_loop_patch_points.pop(); // DO NOT USE `wlf` PAST THIS POINT
}

void
ControlFlowManager::Restricted::mark_forloop_condition_entry()
{
    DEBUG(
        auto &flf_stack = build_ctx_.for_loop_patch_points;
        SMART_ASSERT(!flf_stack.empty());
        SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::BEFORE_CONDITION);
    )
    mark_upcoming_forloop_sites();
}

void
ControlFlowManager::Restricted::mark_forloop_update_list_entry()
{
    DEBUG(
        auto &flf_stack = build_ctx_.for_loop_patch_points;
        SMART_ASSERT(!flf_stack.empty());
        SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::BEFORE_UPDATE_LIST);
    )
    mark_upcoming_forloop_sites();
}

void
ControlFlowManager::Restricted::mark_forloop_update_list_exit(const SourceLocation exit_loc)
{
    DEBUG(
        auto &flf_stack = build_ctx_.for_loop_patch_points;
        SMART_ASSERT(!flf_stack.empty());
        SMART_ASSERT(flf_stack.top().next_patch_point == ForLoopSite::AFTER_UPDATE_LIST);
    )
    mark_upcoming_forloop_sites();
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, exit_loc);
}

void
ControlFlowManager::Restricted::manage_forloop_condition(
    const Expr* conditional,
    const SourceLocation condition_loc)
{
    DEBUG(auto &flf_stack = build_ctx_.for_loop_patch_points;)
    DMASSERT(!!conditional, !flf_stack.empty());
    DMASSERT(flf_stack.top().next_patch_point == ForLoopSite::CONDITION_TRUE);

    conditional = expr_optimizer_->try_propagate_const(conditional);
    conditional = expr_normalizer_->materialize_if_table_item(conditional);
    expr_normalizer_->resolve_bool_short_circuit(conditional);

    mark_upcoming_forloop_sites();
    quad_yielder_->yield_labelless(
        ir::Opcode::IF_EQ,
        nullptr,
        conditional,
        &k_static_true_expr,
        condition_loc
    );

    DMASSERT(flf_stack.top().next_patch_point == ForLoopSite::CONDITION_FALSE);
    mark_upcoming_forloop_sites();
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, condition_loc);
}

void
ControlFlowManager::Restricted::manage_forloop_entry()
{
    DEBUG(auto & flf_stack = build_ctx_.for_loop_patch_points;)
    DMASSERT(!flf_stack.empty());
    // Increment loop counter to keep stack balanced, even if the for-clause is malformed.
    parse_ctx_->func_ctx_handler.enter_loop();

    if (build_ctx_.for_loop_patch_points.top().bad_clause)
        return;

    DMASSERT(flf_stack.top().next_patch_point == ForLoopSite::BEFORE_BODY);
    mark_upcoming_forloop_sites();
}

void
ControlFlowManager::Restricted::manage_forloop_exit(const SourceLocation exit_loc)
{
    DEBUG(auto & flf_stack = build_ctx_.for_loop_patch_points;)
    DMASSERT(!flf_stack.empty());

    auto* const qh = quad_handler_; // Short alias to improve readability.

    const auto& flf = build_ctx_.for_loop_patch_points.top(); // Short alias to improve readability.
    if (!flf.bad_clause)
    {
        // Emit closure loop jump.
        DMASSERT(flf_stack.top().next_patch_point == ForLoopSite::AFTER_BODY);
        mark_upcoming_forloop_sites();
        quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, exit_loc);

        const CodeAddress after_loop_quad_label = qh->next_quad_label(); // First quad outside for-loop.

        qh->labelPatch_quad(flf.condition_true, flf.before_body); // Set IF_EQ true jump inside body
        qh->labelPatch_quad(flf.condition_false, after_loop_quad_label);
        // Set IF_EQ false jump outside body
        qh->labelPatch_quad(flf.after_update_list, flf.before_condition);
        // After update go check condition
        qh->labelPatch_quad(flf.after_body, flf.before_update_list);
        // After closure go update iterators

        // We route all breaks outside the body of the for loop.
        qh->labelPatch_list(parse_ctx_->func_ctx_handler.break_list(), after_loop_quad_label);
        // We route all continues at the beginning of the update_list
        qh->labelPatch_list(parse_ctx_->func_ctx_handler.continue_list(), flf.before_update_list);
    }

    parse_ctx_->func_ctx_handler.exit_loop(); // This kills break and continue lists.
    build_ctx_.for_loop_patch_points.pop();   // DO NOT USE `flf` PAST THIS POINT
}

void
ControlFlowManager::Restricted::enter_forloop_clause()
{
    parse_ctx_->elist_ctx_handler.enter_region(ElistCtxHandler::Region::FORLOOP_CLAUSE);
    build_ctx_.push_new_forloop_patch_point_frame();
}

void
ControlFlowManager::Restricted::exit_forloop_clause()
{
    parse_ctx_->elist_ctx_handler.exit_region(DEBUG(ElistCtxHandler::Region::FORLOOP_CLAUSE));
}

void
ControlFlowManager::Restricted::mark_bad_forloop_clause()
{
    DMASSERT(!build_ctx_.for_loop_patch_points.empty());
    build_ctx_.for_loop_patch_points.top().bad_clause = true;
    parse_ctx_->temp_ctx_handler.reset_temp_ctx_frame();
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
    const Expr* retval)
{
    if (parse_ctx_->func_ctx_handler.function_nesting_depth() == 0)
    {
        dr_->report_return_keyword_outside_func(return_loc);
        return;
    }

    if (retval)
    {
        retval = expr_optimizer_->try_propagate_const(retval);
        retval = expr_normalizer_->materialize_if_table_item(retval);
        expr_normalizer_->resolve_bool_short_circuit(retval);
        quad_yielder_->yield_next(ir::Opcode::RETURN, nullptr, retval, nullptr, return_loc);
    }

    parse_ctx_->func_ctx_handler.add_label_to_returnlist(quad_handler_->next_quad_label());
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, return_loc);
}

void
ControlFlowManager::Restricted::mark_upcoming_forloop_sites()
{
    DMASSERT(!build_ctx_.for_loop_patch_points.empty());
    using FLS = ForLoopSite;

    auto& flf = build_ctx_.for_loop_patch_points.top(); // Short alias to improve readability.

    // clang-format off
    switch (const CodeAddress next_jump_label = quad_handler_->next_quad_label(); flf.next_patch_point)
    {
    case FLS::BEFORE_CONDITION:   flf.before_condition = next_jump_label;   break;
    case FLS::CONDITION_TRUE:     flf.condition_true = next_jump_label;     break;
    case FLS::CONDITION_FALSE:    flf.condition_false = next_jump_label;    break;
    case FLS::BEFORE_UPDATE_LIST: flf.before_update_list = next_jump_label; break;
    case FLS::AFTER_UPDATE_LIST:  flf.after_update_list = next_jump_label;  break;
    case FLS::BEFORE_BODY:        flf.before_body = next_jump_label;        break;
    case FLS::AFTER_BODY:         flf.after_body = next_jump_label;         break;
    default: [[unlikely]] UNREACHABLE(FMT::format(
        "Unknown patch_point: int(patch_point) = {}", static_cast<int>(flf.next_patch_point)));
    }
    // clang-format on

    using UT = std::underlying_type_t<FLS>;
    if (flf.next_patch_point != FLS::AFTER_BODY)
        flf.next_patch_point = static_cast<FLS>(static_cast<UT>(flf.next_patch_point) + 1);
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

    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, keyword_loc);
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

void
ControlFlowManager::commit_forloop_header_expr(const Expr* header_expr)
{
    DMASSERT(!!header_expr);
    const auto& r = restricted();
    DMASSERT(
        r.parse_ctx_->elist_ctx_handler.region().has_value() &&
        r.parse_ctx_->elist_ctx_handler.region().value() == ElistCtxHandler::Region::FORLOOP_CLAUSE
    );

    // TODO: can we remove materialize? can't think of a reason why this would be useful, or even run anytime...
    header_expr = r.expr_normalizer_->materialize_if_table_item(header_expr);
    r.expr_normalizer_->resolve_bool_short_circuit(header_expr);
    r.quad_yielder_->release_temp_handle_if_active(header_expr);
}
} // namespace alpha
