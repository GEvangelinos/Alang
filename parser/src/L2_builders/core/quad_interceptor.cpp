#include "L2_semantic_subsystems/core/quad_interceptor.hpp"

#include "L2_semantic_subsystems/control_flow_manager.hpp"
#include "settings/compiler_settings.hpp"

namespace alpha
{
QuadInterceptor::QuadInterceptor(
    QuadHandler *const quad_handler,
    ParseCtx *const parse_ctx)
    : quad_handler_(support::require_ptr(quad_handler)),
      parse_ctx_(support::require_ptr(parse_ctx)) {}

void
QuadInterceptor::emit(
    const ir::Opcode opc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc,
    const CodeAddress label,
    QuadInterceptor::EmitKey)
{
    const bool is_dead_quad = parse_ctx_->func_ctx_handler.flow_liveness().is_in_dead_flow();
    quad_handler_->emit(opc, result, arg1, arg2, loc, label, is_dead_quad, QuadHandler::EmitKey{});
}
} // namespace alpha
