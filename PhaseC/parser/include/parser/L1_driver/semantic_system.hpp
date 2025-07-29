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
    friend class SemanticSystemBridge;
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
    [[nodiscard]] bool good() const noexcept { return ss_status_ == SemanticSystemStatus::OK; }

private:
    // Gateway class for DiagnosticEngine to set SemanticSystem's error state.
    SemanticSystemStatus ss_status_;
    SemanticSystemGateway ss_gateway_;
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

    // --Layer 2 subsystems -- No trailing underscores here, as these are directly used in dispatch mechanisms.
    AggregateBuilder aggregate_builder;
    AssignBuilder assign_builder;
    BasicBuilder basic_builder;
    BlockManager block_manager;
    ConstBuilder const_builder;
    LoopManager loop_manager;
    LvalueResolver lvalue_resolver;

    // -- Direct methods-- // TODO: maybe package inside a module? // Dont if to unrelatable!
    const Expr *convert_to_bool_expr(const Expr *expr);
    void mark_short_circuit_jump_point();
    void reset_stmt_context() noexcept;
    void finalize_bool_expr(const Expr *expr);
};

DISPATCH_DEFINE_HANDLER_BEGIN(SemanticSystem);
    DISPATCH_BEGIN_CALLS();

    DISPATCH_MASTER_METHOD_CALL(convert_to_bool_expr);
    DISPATCH_MASTER_METHOD_CALL(mark_short_circuit_jump_point);
    DISPATCH_MASTER_METHOD_CALL(reset_stmt_context);
    DISPATCH_MASTER_METHOD_CALL(finalize_bool_expr);
    DISPATCH_MASTER_MODULE_CALL(aggregate_builder);
    DISPATCH_MASTER_MODULE_CALL(assign_builder);
    DISPATCH_MASTER_MODULE_CALL(basic_builder);
    DISPATCH_MASTER_MODULE_CALL(block_manager);
    DISPATCH_MASTER_MODULE_CALL(const_builder);
    DISPATCH_MASTER_MODULE_CALL(loop_manager);
    DISPATCH_MASTER_MODULE_CALL(lvalue_resolver);
    DISPATCH_END_CALLS();
DISPATCH_DEFINE_HANDLER_END(SemanticSystem);


inline void
SemanticSystem::reset_stmt_context() noexcept { parse_ctx_->name_generator.reset_temp_names(); }
} // namespace Alpha
#endif // SEMANTIC_DRIVER_HPP
