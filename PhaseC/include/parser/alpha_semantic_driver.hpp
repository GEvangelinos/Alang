#ifndef ALPHA_SEMANTIC_CONTROLLER_HPP
#define ALPHA_SEMANTIC_CONTROLLER_HPP

#include "core/alpha_basics.hpp"
#include "semantics/expr_builders.hpp"
#include "semantics/expr_validator.hpp"
#include "utils/smart_assert.h"

namespace Alpha
{
struct SemanticOpts
{
        const bool fold_arithmetic;
};

class SemanticDriver : private Immobile
{
private:
        // Must be initialized first -- used by subsystems during their construction.
        // Defaulted to nullptr to trigger safe asserts if construction order is violated.
        SemanticOpts sem_opts_;
        ParseCtx *const parse_ctx_ = nullptr;
        SymbolTable *const symbol_table_ = nullptr;

public:
        // Public subsystems
        ArithmeticBuilder arithmetic_builder;

        SemanticDriver(
                SemanticOpts sem_opts,
                ParseCtx *parse_ctx,
                SymbolTable *symbol_table,
                ErrorTracker *error_tracker);

private:
        // Internal subsystems
        ExprValidator expr_validator_;

        friend class ArithmeticBuilder;
};

inline SemanticDriver::SemanticDriver(
        const SemanticOpts sem_opts,
        ParseCtx *const parse_ctx,
        SymbolTable *const symbol_table,
        ErrorTracker *const error_tracker)
        : sem_opts_(sem_opts),
          parse_ctx_(parse_ctx),
          symbol_table_(symbol_table),
          arithmetic_builder(this),
          expr_validator_(error_tracker)
{
        // Not in hot path. We can afford non DEBUG assertion.
        SMART_ASSERT(!!parse_ctx, !!symbol_table, !!error_tracker);
}
}

#endif //ALPHA_SEMANTIC_CONTROLLER_HPP
