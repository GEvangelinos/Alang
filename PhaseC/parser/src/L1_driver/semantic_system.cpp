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
    : gateway(this),

      // External resources, required to initialize class.
      parse_ctx_(utils::require_ptr(parse_ctx)),
      symbol_table_(utils::require_ptr(symbol_table)),
      dr_(utils::require_ptr(dr)),

      // Private resources, used by public submodules.
      expr_maker_(std::make_unique<ExprMaker>(parse_ctx_)),
      quad_handler_(std::make_unique<QuadHandler>()),
      expr_optimizer_(std::make_unique<ExprOptimizer>(
          get_expr_optimizer_options(options),
          expr_maker_.get()
      )),
      ss_bridge_(parse_ctx_, expr_maker_.get(), quad_handler_.get()),

      // public servicers, used by users of semantic driver.
      aggregate_builder(create_semantic_system_services()),
      assign_builder(get_assign_builder_options(options), create_semantic_system_services()),
      basic_builder(create_semantic_system_services()),
      block_manager(create_semantic_system_services()),
      call_builder(create_semantic_system_services()),
      const_builder(create_semantic_system_services()),
      control_flow_manager(create_semantic_system_services()),
      lvalue_resolver(create_semantic_system_services()),
      function_builder(create_semantic_system_services()),
      table_access_builder(create_semantic_system_services()) {}

SemanticSystemServices
SemanticSystem::create_semantic_system_services()
{
    return {
        .symbol_table = utils::require_ptr(symbol_table_),
        .parse_ctx = utils::require_ptr(parse_ctx_),
        .dr = utils::require_ptr(dr_),
        .expr_maker = utils::require_ptr(expr_maker_.get()),
        .expr_optimizer = utils::require_ptr(expr_optimizer_.get()),
        .quad_handler = utils::require_ptr(quad_handler_.get()),
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
SemanticSystem::reset_stmt_context() noexcept { parse_ctx_->name_generator.reset_temp_names(); }

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
}
} // namespace alpha
