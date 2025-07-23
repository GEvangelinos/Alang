#include "L1_driver/semantic_system.hpp"

namespace Alpha
{
SemanticSystem::SemanticSystem(
    const Options &options,
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    DiagnosticEngine *const diagnostic_engine)
    : ss_gateway_(ss_status_),
      // External resources, required to initialize class.
      parse_ctx_(Utils::require_ptr(parse_ctx)),
      symbol_table_(Utils::require_ptr(symbol_table)),
      diagnostic_engine_(Utils::require_ptr(diagnostic_engine)),

      // private resources, used by public servicers.
      expr_snitch_(std::make_unique<ExprSnitch>(&diagnostic_engine_->dr)),
      expr_maker_(std::make_unique<ExprMaker>(parse_ctx_)),
      expr_folder_(std::make_unique<ExprFolder>(expr_maker_.get(), expr_snitch_.get())),
      quad_handler_(std::make_unique<QuadHandler>()),
      sd_bridge_(parse_ctx_, expr_maker_.get(), quad_handler_.get()),

      // public servicers, used by users of semantic driver.
      backpatcher(export_semantic_system_services()),
      const_builder(export_semantic_system_services()),
      assign_builder(get_assign_builder_options(options), export_semantic_system_services()),
      basic_builder(get_basic_builder_options(options), export_semantic_system_services()),
      loop_manager(export_semantic_system_services()),
      block_manager(export_semantic_system_services()),
      lvalue_resolver(export_semantic_system_services()) {}

SemanticSystemServices
SemanticSystem::export_semantic_system_services()
{
    return SemanticSystemServices{
        .parse_ctx = REQUIRE_PTR(parse_ctx_),
        .symbol_table = REQUIRE_PTR(symbol_table_),
        .dr = &REQUIRE_PTR(diagnostic_engine_)->dr,
        .expr_maker = REQUIRE_PTR(expr_maker_.get()),
        .expr_folder = REQUIRE_PTR(expr_folder_.get()),
        .expr_snitch = REQUIRE_PTR(expr_snitch_.get()),
        .quad_handler = REQUIRE_PTR(quad_handler_.get()),
        .backpatcher = &backpatcher,
        .ss_bridge = &sd_bridge_,
    };
}

AssignBuilder::Options
SemanticSystem::get_assign_builder_options(const Options &options)
{
    return {
        .record_constant_variables = options.propagate_constants
    };
}

BasicBuilder::Options
SemanticSystem::get_basic_builder_options(const Options &options)
{
    return {
        .fold_arithmetic = options.fold_arithmetic,
        .fold_relational = options.fold_relational,
        .fold_logical = options.fold_logical,
        .constant_propagation = options.propagate_constants
    };
}
} // namespace Alpha
