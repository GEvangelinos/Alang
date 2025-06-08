#include "L1_driver/semantic_driver.hpp"

namespace Alpha
{
SemanticDriver::SemanticDriver(
    const Options options,
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    Diagnostics *const diagnostics)
    :
    // External resources, required to initialize class.
    parse_ctx_(Utils::require_ptr(parse_ctx)),
    symbol_table_(Utils::require_ptr(symbol_table)),
    diagnostics_(Utils::require_ptr(diagnostics)),

    // private resources, used by public servicers.
    expr_snitch_(std::make_unique<ExprSnitch>(diagnostics_)),
    expr_maker_(std::make_unique<ExprMaker>(parse_ctx_)),
    expr_folder_(std::make_unique<ExprFolder>(expr_maker_.get(), expr_snitch_.get())),
    quad_handler_(std::make_unique<QuadHandler>()),

    // public servicers, used by users of semantic driver.
    const_builder(expr_maker_.get()),
    basic_builder(get_basic_builder_options(options),
                  expr_snitch_.get(),
                  expr_maker_.get(),
                  expr_folder_.get(),
                  quad_handler_.get(),
                  &parse_ctx_->cache) {}

BasicBuilder::Options
SemanticDriver::get_basic_builder_options(const Options &options)
{
    return {
        .fold_arithmetic = options.fold_arithmetic,
        .fold_relational = options.fold_relational,
        .fold_logical = options.fold_logical,
    };
}
} // namespace Alpha
