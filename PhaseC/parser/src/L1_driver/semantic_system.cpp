#include "L1_driver/semantic_system.hpp"

#include "internal_typedefs.hpp"
#include "core/konstants.hpp"

namespace alpha
{
SemanticSystem::SemanticSystem(
    const Options &options,
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    DiagnosticReporter *const dr)
    : parse_ctx_(support::require_ptr(parse_ctx)),

      // External components, required to initialize class.
      symbol_table_(support::require_ptr(symbol_table)),
      dr_(support::require_ptr(dr)),
      expr_maker_(std::make_unique<ExprMaker>(parse_ctx_)),

      // Private components, used by public submodules.
      quad_handler_(std::make_unique<QuadHandler>()),
      expr_optimizer_(std::make_unique<ExprOptimizer>(
          get_expr_optimizer_options(options),
          expr_maker_.get()
      )),
      ss_bridge_(parse_ctx_, expr_maker_.get(), quad_handler_.get()),
      aggregate_builder(create_semantic_system_services()),

      // public (through call() dispatcher) servicers, used by users of semantic driver.
      assign_builder(get_assign_builder_options(options), create_semantic_system_services()),
      basic_builder(get_basic_builder_options(options), create_semantic_system_services()),
      block_manager(create_semantic_system_services()),
      call_builder(create_semantic_system_services()),
      const_builder(create_semantic_system_services()),
      control_flow_manager(create_semantic_system_services()),
      lvalue_resolver(create_semantic_system_services()),
      function_builder(create_semantic_system_services()),
      table_access_builder(create_semantic_system_services()),

      // public resources used by external components.
      gateway(std::unique_ptr<Gateway>(new Gateway(this))),
      parser_context_view(std::unique_ptr<ParserContextView>(new ParserContextView(this))) {}

SemanticSystemServices
SemanticSystem::create_semantic_system_services()
{
    return {
        .symbol_table = support::require_ptr(symbol_table_),
        .parse_ctx = support::require_ptr(parse_ctx_),
        .dr = support::require_ptr(dr_),
        .expr_maker = support::require_ptr(expr_maker_.get()),
        .expr_optimizer = support::require_ptr(expr_optimizer_.get()),
        .quad_handler = support::require_ptr(quad_handler_.get()),
        .ss_bridge = &ss_bridge_,
    };
}

AssignBuilder::Options
SemanticSystem::get_assign_builder_options(const Options &options)
{
    return {
        // constant propagation requires recording of constants inside Expr(essions)
        .record_constant_variables = options.propagate_constants
    };
}

BasicBuilder::Options
SemanticSystem::get_basic_builder_options(const Options &options)
{
    return {
        .fold_static_bools = options.expr_folding
    };
}

ExprOptimizer::Options
SemanticSystem::get_expr_optimizer_options(const Options &options)
{
    return {
        .constant_propagation = options.propagate_constants,
        .expr_folding = options.expr_folding,
        .expr_trimming = options.expr_trimming,
    };
}

void
SemanticSystem::reset_stmt_context() noexcept
{
    parse_ctx_->temp_ctx_handler.reset_current_frame();
}

void
SemanticSystem::consume_stmt_expr(const Expr *const expr)
{
    const Expr *const materialized_expr = ss_bridge_.materialize_if_table_item(expr);
    ss_bridge_.finalize_bool_expr(materialized_expr);
}

const Expr *
SemanticSystem::force_rvalue_cast(const Expr *const expr, const SourceLocation cast_loc)
{
    const Expr *const result = expr_maker_->clone_with_updated_location(cast_loc, expr);
    result->rvalue_cast();
    return result;
}

void
SemanticSystem::Gateway::notify_hard_error() noexcept
{
    host_->ss_status_ = SemanticSystem::Status::ERROR;
    host_->parse_ctx_->hard_error_occurred.raise();
}

SemanticSystem::ParserContextView::ParserContextView(SemanticSystem *const ss)
    : host_(support::require_ptr(ss)) {}

bool SemanticSystem::ParserContextView::is_in_func_param_list() const noexcept
{
    return host_->parse_ctx_->space_handler.space() == VarSymbol::Space::FORMAL_ARGUMENT;
}

bool SemanticSystem::ParserContextView::is_in_call_arg_list() const noexcept
{
    return host_->parse_ctx_->call_ctx_handler.is_in_call();
}

bool SemanticSystem::ParserContextView::is_in_forloop_clause() const noexcept
{
    return host_->parse_ctx_->temp_ctx_handler.current_critical_region() ==
           TempCtxHandler::CriticalRegion::FORLOOP_CLAUSE;
}
} // namespace alpha
