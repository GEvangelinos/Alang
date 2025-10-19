#ifndef QUAD_INTERCEPTOR_HPP
#define QUAD_INTERCEPTOR_HPP

#include "L2_semantic_subsystems/core/quad_handler.hpp"

namespace alpha
{
class ControlFlowManager;

// clang-format off
namespace settings { struct IROpts; } // clang-format on

class QuadInterceptor
{
public:
    class EmitKey // Passkey mechanisms, so only friends to EmitKey can emit
    {
        friend class QuadYielder;
        friend class ExprNormalizer;
        EmitKey() = default;
    };

    QuadInterceptor(settings::IROpts ir_opts, QuadHandler *quad_handler);
    void attach_control_flow_manager(ControlFlowManager * control_flow_manger);

    void emit(
        ir::Opcode opc,
        const Expr *result,
        const Expr *arg1,
        const Expr *arg2,
        SourceLocation loc,
        LabelID label,
        QuadInterceptor::EmitKey);

private:
    const bool eliminate_dead_code_;
    QuadHandler *const quad_handler_;
    Once<ControlFlowManager *> control_flow_manager_;
};
} // namespace alpha
#endif // QUAD_INTERCEPTOR_HPP
