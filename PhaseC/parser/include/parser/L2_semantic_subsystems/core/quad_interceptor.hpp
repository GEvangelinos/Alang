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

    QuadInterceptor(QuadHandler *quad_handler, ParseCtx *parse_ctx);

    void emit(
        ir::Opcode opc,
        const Expr *result,
        const Expr *arg1,
        const Expr *arg2,
        SourceLocation loc,
        LabelID label,
        QuadInterceptor::EmitKey);

private:
    QuadHandler *const quad_handler_;
    ParseCtx *const parse_ctx_;
};
} // namespace alpha
#endif // QUAD_INTERCEPTOR_HPP
