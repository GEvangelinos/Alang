#ifndef SEMANTIC_DRIVER_HPP
#define SEMANTIC_DRIVER_HPP

#include "parser/parser_context.hpp"
#include "parser/symbol_table.hpp"
#include "core/basics.hpp"
#include "core/diagnostics.hpp"
#include "L2_builders/expr_builders.hpp"
#include "L3_ir_infra/expr_folder.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/expr_snitch.hpp"
#include "L3_ir_infra/quad_handler.hpp"

namespace Alpha
{
// Order of initialization is intentional.
// As the subsystems of semantic driver utilize its internal state,
// for their own initialization.
class SemanticDriver : private Immobile
{
public:
    struct Options
    {
        const bool fold_arithmetic;
        const bool fold_relational;
        const bool fold_logical;
    };

    SemanticDriver(
        Options options,
        ParseCtx *parse_ctx,
        SymbolTable *symbol_table,
        Diagnostics *diagnostics);

    // TODO make a function that user calls before destructor call that basically extracts all this
    // alpha drivers would want (like the generated quads).
    [[nodiscard]] const std::vector<Quad> &retrieve_quads() const noexcept
    {
        return quad_handler_->quads();
    }

private:
    // Must be initialized first -- used by subsystems during their construction.
    // Defaulted to nullptr to trigger safe asserts if construction order is violated.
    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    Diagnostics *const diagnostics_ = nullptr;

    // -- Layer 3 subsystems --
    std::unique_ptr<ExprSnitch> expr_snitch_;
    std::unique_ptr<ExprMaker> expr_maker_;
    std::unique_ptr<ExprFolder> expr_folder_;
    std::unique_ptr<QuadHandler> quad_handler_;

public:
    // --Layer 2 subsystems --
    ConstBuilder const_builder;
    BasicBuilder basic_builder;

    static BasicBuilder::Options get_basic_builder_options(const Options &options);
};
} // namespace Alpha
#endif // SEMANTIC_DRIVER_HPP
