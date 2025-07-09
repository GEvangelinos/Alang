#include "L1_driver/semantic_driver.hpp"

#include "diagnostics/diagnostic_reporter.gen.hpp"

namespace Alpha
{
SemanticDriver::SemanticDriver(
    const Options options,
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    DiagnosticEngine *const diagnostic_engine)
    :
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
    const_builder(make_builder_init_pack()),
    basic_builder(get_basic_builder_options(options), make_builder_init_pack()),
    assign_builder(make_builder_init_pack()),
    lvalue_resolver(make_builder_init_pack()) {}

DriverInitPack
SemanticDriver::make_builder_init_pack()
{
    return DriverInitPack{
        .parse_ctx = REQUIRE_PTR(parse_ctx_),
        .symbol_table = REQUIRE_PTR(symbol_table_),
        .dr = &REQUIRE_PTR(diagnostic_engine_)->dr,
        .expr_maker = REQUIRE_PTR(expr_maker_.get()),
        .expr_folder = REQUIRE_PTR(expr_folder_.get()),
        .expr_snitch = REQUIRE_PTR(expr_snitch_.get()),
        .quad_handler = REQUIRE_PTR(quad_handler_.get()),
        .sd_bridge = &sd_bridge_,
    };
}

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
