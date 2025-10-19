#include "L2_semantic_subsystems/core/quad_interceptor.hpp"

#include "L2_semantic_subsystems/control_flow_manager.hpp"
#include "settings/compiler_settings.hpp"

namespace alpha
{
QuadInterceptor::QuadInterceptor(
    const settings::IROpts ir_opts,
    QuadHandler *const quad_handler)
    : eliminate_dead_code_(ir_opts.opt_dead_code_elimination),
      quad_handler_(support::require_ptr(quad_handler)),
    control_flow_manager_()
      {}

void
QuadInterceptor::attach_control_flow_manager(ControlFlowManager *const control_flow_manger)
{
    control_flow_manager_ = support::require_ptr(control_flow_manger);
}

void
QuadInterceptor::emit(
    const ir::Opcode opc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc,
    const LabelID label,
    QuadInterceptor::EmitKey)
{
    DEBUG_SMART_ASSERT(control_flow_manager_.is_assigned() && "ControlFlowManager is not bind yet");

    if (eliminate_dead_code_)
        if (control_flow_manager_->is_in_dead_block())
            return;

    quad_handler_->emit(opc, result, arg1, arg2, loc, label, QuadHandler::EmitKey{});
}
} // namespace alpha
