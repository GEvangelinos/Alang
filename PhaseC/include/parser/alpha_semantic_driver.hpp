#ifndef ALPHA_SEMANTIC_CONTROLLER_HPP
#define ALPHA_SEMANTIC_CONTROLLER_HPP

#include "core/alpha_basics.hpp"
#include "semantics/expr_folder.hpp"
#include "semantics/expr_maker.hpp"
#include "semantics/expr_validation.hpp"

namespace Alpha
{
struct SemanticOpts
{
        const bool fold_arithmetic;
};

// Order of initialization is intentionally first private then public.
// As the subsystems of semantic driver utilize its internal state,
// for their own initialization.
class SemanticDriver : private Immobile
{
private:
        // Must be initialized first -- used by subsystems during their construction.
        // Defaulted to nullptr to trigger safe asserts if construction order is violated.
        SemanticOpts sem_opts_;
        ParseCtx *const parse_ctx_ = nullptr;
        SymbolTable *const symbol_table_ = nullptr;
        ErrorTracker *const error_tracker_ = nullptr;

        // Internal subsystems
        ExprFolder expr_folder_;
        ExprValidator expr_validator_;

        friend class ExprFolder;
        friend class ExprMaker;
public:
        // Public subsystems
        ExprMaker expr_maker;

        SemanticDriver(SemanticOpts sem_opts, ParseCtx *parse_ctx,
                       SymbolTable *symbol_table, ErrorTracker *error_tracker);
};

inline SemanticDriver::SemanticDriver(
        const SemanticOpts sem_opts,
        ParseCtx *const parse_ctx,
        SymbolTable *const symbol_table,
        ErrorTracker *const error_tracker)
        : sem_opts_(sem_opts),
          parse_ctx_(Utils::require_ptr(parse_ctx)),
          symbol_table_(Utils::require_ptr(symbol_table)),
          error_tracker_(Utils::require_ptr(error_tracker)),
          expr_validator_(Utils::require_ptr(error_tracker)),
          arithmetic_builder(this),
          expr_folder_(this) {}
}

#endif //ALPHA_SEMANTIC_CONTROLLER_HPP
