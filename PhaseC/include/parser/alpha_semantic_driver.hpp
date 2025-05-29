#ifndef ALPHA_SEMANTIC_DRIVER_HPP
#define ALPHA_SEMANTIC_DRIVER_HPP

#include "core/alpha_basics.hpp"
#include "semantics/expr_builders.hpp"
#include "semantics/expr_folder.hpp"
#include "semantics/expr_maker.hpp"
#include "semantics/expr_snitch.hpp"
#include "semantics/quad_handler.hpp"

namespace Alpha
{
// Order of initialization is intentionally first private then public.
// As the subsystems of semantic driver utilize its internal state,
// for their own initialization.
class SemanticDriver : private Immobile
{
public:
    struct Options
    {
        const bool fold_arithmetic;
        const bool fold_bool;
    };

    SemanticDriver(
        Options options,
        ParseCtx *parse_ctx,
        SymbolTable *symbol_table,
        Diagnostics *diagnostics);

private:
    // Must be initialized first -- used by subsystems during their construction.
    // Defaulted to nullptr to trigger safe asserts if construction order is violated.
    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    Diagnostics *const diagnostics_ = nullptr;

    // Internal layer 3 subsystems
    ExprSnitch expr_validator_;
    ExprMaker expr_maker_;
    ExprFolder expr_folder_;
    QuadHandler quad_handler;

    // Public  layer 2 subsystems
    BasicBuilder basic_builder_;

    static BasicBuilder::Options extract_basic_builder_options(const Options &options);
};

inline SemanticDriver::SemanticDriver(
    const Options options,
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    Diagnostics *const diagnostics)
    : parse_ctx_(Utils::require_ptr(parse_ctx)),
      symbol_table_(Utils::require_ptr(symbol_table)),
      diagnostics_(Utils::require_ptr(diagnostics)),
      expr_validator_(Utils::require_ptr(diagnostics)),
      expr_maker_(parse_ctx),
      expr_folder_(&expr_maker_, diagnostics),
      basic_builder_(extract_basic_builder_options(options),
                     &expr_validator_,
                     &expr_maker_,
                     &expr_folder_,
                     &quad_handler) {}

// Options &&options,
// ExprSnitch *expr_validator,
// ExprMaker *expr_maker,
// ExprFolder *expr_folder);
inline BasicBuilder::Options
SemanticDriver::extract_basic_builder_options(const Options &options)
{
    return {
        .fold_arithmetic = options.fold_arithmetic,
        .fold_bool = options.fold_bool
    };
}
} // namespace Alpha
#endif // ALPHA_SEMANTIC_DRIVER_HPP
