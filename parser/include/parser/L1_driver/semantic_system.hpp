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

#ifndef SEMANTIC_SYSTEM_HPP
#define SEMANTIC_SYSTEM_HPP

#include <core/fixed_string.hpp>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>

#include "core/basics.hpp"
#include "L2_semantic_subsystems/block_manager.hpp"
#include "L2_semantic_subsystems/core/expr_maker.hpp"
#include "L2_semantic_subsystems/core/expr_optimizer.hpp"
#include "L2_semantic_subsystems/core/quad_handler.hpp"
#include "L2_semantic_subsystems/core/expr_normalizer.hpp"
#include "parser/parser_context.hpp"
#include "parser/symbol_table.hpp"
#include "parser/L2_semantic_subsystems/control_flow_manager.hpp"
#include "parser/L2_semantic_subsystems/expr_builders.hpp"
#include "parser/L2_semantic_subsystems/lvalue_resolver.hpp"
#include "settings/compiler_settings.hpp"

namespace alpha
{
class SemanticSystem : private Immobile
{
    friend class SemanticSystemBridge;

public: // More public stuff at the end (check it out)
    SemanticSystem(
        const settings::ExprOpts &expr_opts,
        ParseCtx *parse_ctx,
        SymbolTable *symbol_table,
        DiagnosticReporter &dr);

    // TODO: make a function that user calls before destructor call that basically extracts all this
    // alpha drivers would want (like the generated quads).

    void recover() noexcept { ss_status_ = Status::OK; }
    [[nodiscard]] bool good() const noexcept { return ss_status_ == Status::OK; }


    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_MASTER_MODULE_CALL(assign_builder);
    DISPATCH_MASTER_MODULE_CALL(basic_builder);
    DISPATCH_MASTER_MODULE_CALL(block_manager);
    DISPATCH_MASTER_MODULE_CALL(call_builder);
    DISPATCH_MASTER_MODULE_CALL(const_builder);
    DISPATCH_MASTER_MODULE_CALL(control_flow_manager);
    DISPATCH_MASTER_MODULE_CALL(lvalue_resolver);
    DISPATCH_MASTER_MODULE_CALL(function_builder);
    DISPATCH_MASTER_MODULE_CALL(table_access_builder);
    DISPATCH_MASTER_MODULE_CALL(aggregate_builder);
    DISPATCH_MASTER_METHOD_CALL(reset_stmt_context);
    DISPATCH_MASTER_METHOD_CALL(consume_stmt_expr);
    DISPATCH_MASTER_METHOD_CALL(commit_expr_of_elist);
    DISPATCH_MASTER_METHOD_CALL(force_rvalue_cast);
    DISPATCH_DEFINE_HANDLER_END();

private:
    enum class Status : u8 { OK, ERROR };

    Status ss_status_ = Status::OK;

    // Must be initialized first -- used by subsystems during their construction.
    // Defaulted to nullptr to trigger safe asserts if construction order is violated.

    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    DiagnosticReporter *const dr_ = nullptr;

    // -- Layer 3 core subsystems -- We use unique_ptr instead of normal vars, in order to detect wrong initialization order
    std::unique_ptr<ExprMaker> expr_maker_;
    std::unique_ptr<QuadHandler> quad_handler_;
    std::unique_ptr<QuadInterceptor> quad_interceptor_;
    std::unique_ptr<QuadYielder> quad_yielder_;
    std::unique_ptr<ExprNormalizer> expr_normalizer_;
    std::unique_ptr<ExprOptimizer> expr_optimizer_;

    // -- Layer 2 subsystems -- No trailing underscores here, as these are directly used in DSL dispatcher
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

    // -- Directly dispatchable  methods-- // TODO: maybe package inside a module?
    void reset_stmt_context() noexcept;
    void consume_stmt_expr(const Expr *expr);
    void commit_expr_of_elist(const Expr *expr);
    [[nodiscard]] const Expr *force_rvalue_cast(const Expr *expr, SourceLocation cast_loc);

    [[nodiscard]] SemanticSystemServices create_semantic_system_services();

    [[nodiscard]] static AssignBuilder::Options get_assign_builder_options(
        const settings::ExprOpts &expr_opts);
    [[nodiscard]] static BasicBuilder::Options get_basic_builder_options(
        const settings::ExprOpts &expr_opts);

public:
    // Gateway lets PassManager mark hard errors, but not clear them;
    // recovery hooks via call() dispatch can still reset, so it’s not bulletproof.
    // Gateway also provides access to the generated quads.
    class Gateway;

    // Used to give access to specific queries to Bison's custom syntax error handler
    class ContextInspector;

    std::unique_ptr<Gateway> gateway;
    std::unique_ptr<ContextInspector> context_inspector;
};

class SemanticSystem::ContextInspector
{
    friend class SemanticSystem;

public:
    [[nodiscard]] bool is_in_call_arg_list() const noexcept;
    [[nodiscard]] bool is_in_forloop_clause() const noexcept;
    [[nodiscard]] bool is_in_func_param_list() const noexcept;
    [[nodiscard]] bool is_in_table_dict() const noexcept;

private:
    SemanticSystem *const host_;

    explicit ContextInspector(SemanticSystem *ss);
};

// Gateway lets PassManager mark hard errors, but not clear them;
// recovery hooks via call() dispatch can still reset, so it’s not bulletproof.
// Gateway also provides access to the generated quads.
class SemanticSystem::Gateway
{
    friend class CompilationPipeline;
    friend class SemanticSystem;

private:
    SemanticSystem *const host_;
    OnceFlag extracted_quads;

    explicit Gateway(SemanticSystem *ss);

    void notify_hard_error() noexcept;

    [[nodiscard]] ir::QuadStream extract_quads();
};
} // namespace alpha
#endif // SEMANTIC_SYSTEM_HPP
