#ifndef SEMANTIC_DRIVER_HPP
#define SEMANTIC_DRIVER_HPP

#include <core/fixed_string.hpp>

#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include "core/basics.hpp"
#include "L1_driver/semantic_system_gateway.hpp"
#include "L2_builders/control_flow_managers.hpp"
#include "L2_builders/expr_builders.hpp"
#include "L2_builders/lvalue_resolver.hpp"
#include "L3_ir_infra/expr_folder.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/expr_snitch.hpp"
#include "L3_ir_infra/quad_handler.hpp"
#include "parser/parser_context.hpp"
#include "parser/symbol_table.hpp"


namespace Alpha
{
// Order of initialization is intentional.
// As the subsystems of semantic driver utilize its internal state,
// for their own initialization.
class SemanticSystem : private Immobile
{
public:
    struct Options
    {
        const bool fold_arithmetic;
        const bool fold_relational;
        const bool fold_logical;
        const bool propagate_constants;
        const bool propagate_const_return;
    };

    SemanticSystem(
        const Options &options,
        ParseCtx *parse_ctx,
        SymbolTable *symbol_table,
        DiagnosticEngine *diagnostic_engine);

    DISPATCH_DECLARE_HANDLER();

    // TODO make a function that user calls before destructor call that basically extracts all this
    // alpha drivers would want (like the generated quads).
    [[nodiscard]] const auto &retrieve_quads() const noexcept { return quad_handler_->quads(); }

private:
    // Gateway class for DiagnosticEngine to set SemanticSystem's error state.
    bool in_semantic_error = false;
    SemanticSystemGateway error_gateway_;
    // Must be initialized first -- used by subsystems during their construction.
    // Defaulted to nullptr to trigger safe asserts if construction order is violated.
    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    DiagnosticEngine *const diagnostic_engine_ = nullptr;
    // -- Layer 3 subsystems --
    // We use unique_ptr instead of normal vars, in order to detect wrong initialization order
    std::unique_ptr<ExprSnitch> expr_snitch_;
    std::unique_ptr<ExprMaker> expr_maker_;
    std::unique_ptr<ExprFolder> expr_folder_;
    std::unique_ptr<QuadHandler> quad_handler_;
    SemanticSystemBridge sd_bridge_;

    SemanticSystemServices export_semantic_system_services();

    static AssignBuilder::Options get_assign_builder_options(const Options &options);
    static BasicBuilder::Options get_basic_builder_options(const Options &options);

    friend class SemanticSystemBridge;

    // --Layer 2 subsystems --
    Backpatcher backpatcher;
    ConstBuilder const_builder;
    AssignBuilder assign_builder;
    BasicBuilder basic_builder;
    LoopManager loop_manager;
    LvalueResolver lvalue_resolver;

    // -- Direct methods-- // TODO: maybe package inside a module? // Dont if to unrelatable!
    void mark_short_circuit_jump_point();
    const Expr *convert_to_bool_expr(const Expr *expr);
    void reset_stmt_context() noexcept;
};

DISPATCH_DEFINE_HANDLER_BEGIN(SemanticSystem);
    DISPATCH_BEGIN_CALLS();
    DISPATCH_CALL_METHOD(mark_short_circuit_jump_point);
    DISPATCH_CALL_METHOD(convert_to_bool_expr);
    DISPATCH_CALL_METHOD(reset_stmt_context);
    DISPATCH_CALL_MODULE(backpatcher);
    DISPATCH_CALL_MODULE(const_builder);
    DISPATCH_CALL_MODULE(assign_builder);
    DISPATCH_CALL_MODULE(basic_builder);
    DISPATCH_CALL_MODULE(loop_manager);
    DISPATCH_CALL_MODULE(lvalue_resolver);
    DISPATCH_END_CALLS();
DISPATCH_DEFINE_HANDLER_END(SemanticSystem);

inline void
SemanticSystem::mark_short_circuit_jump_point()
{
    parse_ctx_->cache.short_circuit_jump_stack.push(quad_handler_->next_quad_label());
}

// Conversion to bool was IF_EQ and then JUMP, to optimize, I went with a single IF_NOTEQ
inline const Expr *
SemanticSystem::convert_to_bool_expr(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);

    if (expr->type == Expr::Type::BOOL_EXPR)
        return expr;
    if (SemUtils::is_static_expr(expr))
        return SemUtils::as_bool(expr)
               ? expr_maker_->make_const_bool_expr(expr->loc, true)
               : expr_maker_->make_const_bool_expr(expr->loc, false);

    const BoolExpr *const bool_expr = expr_maker_->make_bool_expr(expr->loc);
    bool_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(IOPCode::IF_EQ, expr, &k_static_true_expr, nullptr, expr->loc);
    bool_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, expr->loc);

    return bool_expr;
}

inline void
SemanticSystem::reset_stmt_context() noexcept
{
    parse_ctx_->name_generator.reset_temp_names();
}
} // namespace Alpha
#endif // SEMANTIC_DRIVER_HPP
