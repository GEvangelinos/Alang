#ifndef QUAD_YIELDER_HPP
#define QUAD_YIELDER_HPP

#include "expr_maker.hpp"
#include "expr_optimizer.hpp"
#include "core/ir/ir_expr.hpp"
#include "parser_context.hpp"
#include "quad_handler.hpp"
#include "quad_interceptor.hpp"

namespace alpha
{
template <typename Callable>
concept ExprFactory =
    std::is_pointer_v<std::invoke_result_t<Callable>> &&
    std::is_const_v<std::remove_pointer_t<std::invoke_result_t<Callable>>> &&
    std::is_base_of_v
    <
        Expr,
        std::remove_const_t<std::remove_pointer_t<std::invoke_result_t<Callable>>>
    >;

class QuadYielder
{
public:
    QuadYielder(
        ParseCtx* parse_ctx,
        SymbolTable* symbol_table,
        ExprMaker* expr_maker,
        QuadHandler* quad_handler,
        QuadInterceptor* quad_interceptor);

    void release_temp_handle_if_active(const Expr* expr);

    /// @returns result_factory's retval
    template <ExprFactory ResultFactory>
    const Expr* yield(
        ir::Opcode opc,
        ResultFactory&& result_factory,
        const Expr* arg1,
        const Expr* arg2,
        SourceLocation loc,
        CodeAddress label);

    /// @returns result
    const Expr* yield(
        ir::Opcode opc,
        const Expr* result,
        const Expr* arg1,
        const Expr* arg2,
        SourceLocation loc,
        CodeAddress label);

    template <ExprFactory PreEmitHook>
    const Expr* yield_returning_hook_result(
        ir::Opcode opc,
        const Expr* result,
        const Expr* arg1,
        const Expr* arg2,
        SourceLocation loc,
        CodeAddress label,
        PreEmitHook&& pre_emit_hook);

    /// @returns result_factory's return value
    template <ExprFactory ResultFactory>
    const Expr* yield_next(
        ir::Opcode opc,
        ResultFactory&& result_factory,
        const Expr* arg1,
        const Expr* arg2,
        SourceLocation loc,
        CodeAddress label_offset = CodeAddress{0}
    );

    /// @returns param result
    const Expr* yield_next(
        ir::Opcode opc,
        const Expr* result,
        const Expr* arg1,
        const Expr* arg2,
        SourceLocation loc,
        CodeAddress label_offset = CodeAddress{0}
    );

    /// @returns result_factory's return value
    template <ExprFactory ResultFactory>
    const Expr* yield_labelless(
        ir::Opcode opc,
        ResultFactory&& result_factory,
        const Expr* arg1,
        const Expr* arg2,
        SourceLocation loc);

    const Expr* yield_labelless(
        ir::Opcode opc,
        const Expr* result,
        const Expr* arg1,
        const Expr* arg2,
        SourceLocation loc);

private:
    ParseCtx* const parse_ctx_;
    SymbolTable* const symbol_table_;
    ExprMaker* const expr_maker_;
    QuadHandler* const quad_handler_;
    QuadInterceptor* const quad_interceptor_;
};

template <ExprFactory ResultFactory>
const Expr*
QuadYielder::yield(
    const ir::Opcode opc,
    ResultFactory&& result_factory,
    const Expr* arg1,
    const Expr* arg2,
    const SourceLocation loc,
    const CodeAddress label)
{
    // Release temps before making result (which usually creates a new temp)
    // Order first arg2 then arg1 is mandatory, in release, as acquiring happens in reverse.
    if (arg2) release_temp_handle_if_active(arg2);
    if (arg1) release_temp_handle_if_active(arg1);

    const Expr* const result = std::forward<ResultFactory>(result_factory)();
    quad_interceptor_->emit(opc, result, arg1, arg2, loc, label, QuadInterceptor::EmitKey{});
    return result;
}

template <ExprFactory PreEmitHook>
const Expr*
QuadYielder::yield_returning_hook_result(
    const ir::Opcode opc,
    const Expr* const result,
    const Expr* const arg1,
    const Expr* const arg2,
    const SourceLocation loc,
    const CodeAddress label,
    PreEmitHook&& pre_emit_hook)
{
    // Release temps before making result (which usually creates a new temp)
    // Order first arg2 then arg1 is mandatory, in release, as acquiring happens in reverse.
    if (arg2) release_temp_handle_if_active(arg2);
    if (arg1) release_temp_handle_if_active(arg1);

    const Expr* const hook_result = std::forward<PreEmitHook>(pre_emit_hook)();
    quad_interceptor_->emit(opc, result, arg1, arg2, loc, label, QuadInterceptor::EmitKey{});
    return hook_result;
}

template <ExprFactory ResultFactory>
const Expr*
QuadYielder::yield_next(
    const ir::Opcode opc,
    ResultFactory&& result_factory,
    const Expr* const arg1,
    const Expr* const arg2,
    const SourceLocation loc,
    const CodeAddress label_offset)
{
    return yield(
        opc,
        std::forward<ResultFactory>(result_factory),
        arg1,
        arg2,
        loc,
        quad_handler_->next_quad_label() + label_offset
    );
}

/// @returns result_factory's return value
template <ExprFactory ResultFactory>
const Expr*
QuadYielder::yield_labelless(
    const ir::Opcode opc,
    ResultFactory&& result_factory,
    const Expr* const arg1,
    const Expr* const arg2,
    const SourceLocation loc)
{
    return yield(
        opc,
        std::forward<ResultFactory>(result_factory),
        arg1,
        arg2,
        loc,
        CodeAddress::none()
    );
}
} // namespace alpha
#endif //QUAD_YIELDER_HPP
