/**
* SemanticSystem overview
 *
 * Acts as the top-level coordinator for all semantic actions. It owns/initializes the
 * L3 infrastructure (ExprMaker/QuadHandler/ExprOptimizer) and the L2 subsystems
 * (Aggregate/Assign/Basic/Block/Const/Loop/Lvalue). The class exposes a single
 * compile-time dispatcher `call<"module.method">(args...)` (see DSL below).
 *
 * The dispatcher routes strings like "basic_builder.build_arithmetic" to the
 * corresponding subsystem method, while first checking `good()` to short-circuit
 * error paths. Only the dispatcher is “friended” by the parser; each subsystem
 * hides its internals in a nested `Restricted` class (friendship is non-commutative),
 * so SemanticSystem exposes only what its Builders choose to surface.
 * ------------------------------------------------------------------------------------
 * Layer 3 subsystems — stored as unique_ptr on purpose:
 *  - detects bad initialization order at runtime: semantic-subsystem-based
 *    builders call require_ptr(...) in their constructor to assert these core services
 *    are set before use.
 *  - no performance loss versus references: even a reference is compiled as
 *    an underlying const pointer, so there’s still one indirection.
 *  - explicit dereference syntax (`ptr->member`) makes it obvious when an
 *    access involves a pointer, unlike references where `obj.member` hides it.
 */

#ifndef SEMANTIC_DRIVER_HPP
#define SEMANTIC_DRIVER_HPP

#include <core/fixed_string.hpp>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include "core/basics.hpp"
#include "L1_driver/semantic_system_gateway.hpp"
#include "L2_semantic_subsystems/block_manager.hpp"
#include "parser/L2_semantic_subsystems/control_flow_managers.hpp"
#include "parser/L2_semantic_subsystems/expr_builders.hpp"
#include "parser/L2_semantic_subsystems/lvalue_resolver.hpp"
#include "L3_ir_infra/expr_optimizer.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/quad_handler.hpp"
#include "parser/parser_context.hpp"
#include "parser/symbol_table.hpp"

namespace alpha
{
// Order of initialization is intentional.
// As the subsystems of semantic driver utilize its internal state,
// for their own initialization.
class SemanticSystem : private Immobile
{
    friend class SemanticSystemBridge;

public:
    struct Options
    {
        const bool expr_folding;
        const bool expr_trimming;
        const bool propagate_constants;
        const bool propagate_const_return;
    };

    SemanticSystem(
        const Options &options,
        ParseCtx *parse_ctx,
        SymbolTable *symbol_table,
        DiagnosticEngine *diagnostic_engine);

    // TODO: make a function that user calls before destructor call that basically extracts all this
    // alpha drivers would want (like the generated quads).
    [[nodiscard]] const auto &retrieve_quads() const noexcept { return quad_handler_->quads(); }
    [[nodiscard]] bool good() const noexcept { return ss_status_ == SemanticSystemStatus::OK; }

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_MASTER_MODULE_CALL(aggregate_builder);
    DISPATCH_MASTER_MODULE_CALL(assign_builder);
    DISPATCH_MASTER_MODULE_CALL(basic_builder);
    DISPATCH_MASTER_MODULE_CALL(block_manager);
    DISPATCH_MASTER_MODULE_CALL(call_builder);
    DISPATCH_MASTER_MODULE_CALL(const_builder);
    DISPATCH_MASTER_MODULE_CALL(control_flow_manager);
    DISPATCH_MASTER_MODULE_CALL(lvalue_resolver);
    DISPATCH_MASTER_MODULE_CALL(function_builder);
    DISPATCH_MASTER_MODULE_CALL(table_access_builder);
    DISPATCH_MASTER_METHOD_CALL(convert_to_bool_expr);
    DISPATCH_MASTER_METHOD_CALL(mark_short_circuit_jump_point);
    DISPATCH_MASTER_METHOD_CALL(reset_stmt_context);
    DISPATCH_MASTER_METHOD_CALL(finalize_bool_expr);
    DISPATCH_DEFINE_HANDLER_END();

private:
    SemanticSystemStatus ss_status_ = SemanticSystemStatus::OK;

    // Gateway class for DiagnosticEngine to set SemanticSystem's error state.
    SemanticSystemGateway ss_gateway_;

    // Must be initialized first -- used by subsystems during their construction.
    // Defaulted to nullptr to trigger safe asserts if construction order is violated.
    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    DiagnosticEngine *const diagnostic_engine_ = nullptr;

    // -- Layer 3 subsystems -- We use unique_ptr instead of normal vars, in order to detect wrong initialization order
    std::unique_ptr<ExprMaker> expr_maker_;
    std::unique_ptr<QuadHandler> quad_handler_;
    std::unique_ptr<ExprOptimizer> expr_optimizer_;
    SemanticSystemBridge sd_bridge_;

    // -- Layer 2 subsystems -- No trailing underscores here, as these are directly used in dispatch mechanisms.
    AggregateBuilder aggregate_builder;
    AssignBuilder assign_builder;
    BasicBuilder basic_builder;
    BlockManager block_manager;
    CallBuilder call_builder;
    ConstBuilder const_builder;
    ControlFlowManager control_flow_manager;
    LvalueResolver lvalue_resolver;
    FunctionBuilder function_builder;
    TableAccessBuilder table_access_builder;

    // -- Direct methods-- // TODO: maybe package inside a module? // Dont if to unrelatable!
    const Expr *convert_to_bool_expr(const Expr *expr);
    void reset_stmt_context() noexcept;
    void finalize_bool_expr(const Expr *expr);

    SemanticSystemServices export_semantic_system_services();

    static AssignBuilder::Options get_assign_builder_options(const Options &options);
    static ExprOptimizer::Options get_expr_optimizer_options(const Options &options);
};

inline void
SemanticSystem::reset_stmt_context() noexcept { parse_ctx_->name_generator.reset_temp_names(); }
} // namespace alpha
#endif // SEMANTIC_DRIVER_HPP
