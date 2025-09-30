#ifndef SEMANTIC_SUBSYSTEM_HPP
#define SEMANTIC_SUBSYSTEM_HPP

#include <L1_driver/semantic_system_support.hpp>

namespace alpha
{
class SemanticSubsystem : private Immobile
{
protected:
    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    DiagnosticReporter *const dr_ = nullptr;
    ExprMaker *const expr_maker_ = nullptr;
    ExprOptimizer *const expr_optimizer_ = nullptr;
    QuadHandler *const quad_handler_ = nullptr;
    SemanticSystemBridge *const ss_bridge_ = nullptr;

    explicit SemanticSubsystem(const SemanticSystemServices &ss_services);

    void release_temp_handle_if_active(const Expr *unary);
    void release_temp_handle_if_active(const Expr *lhs, const Expr *rhs);

    virtual ~SemanticSubsystem() = 0;
};

inline void
SemanticSubsystem::release_temp_handle_if_active(const Expr *const unary)
{
    // Rvalue operands don't persist, so temp names can be safely reused.
    if (unary->has_active_temp())
    {
        const VarSymbol* const var_symbol =  static_cast<const ExprWVarSymbol *>(unary)->var_symbol;
        parse_ctx_->temp_ctx_handler.release_temp_handle(var_symbol->temp_handle());
        symbol_table_->detach_temp_handle(var_symbol);
    }
}

inline void
SemanticSubsystem::release_temp_handle_if_active(const Expr *const lhs, const Expr *const rhs)
{
    /// @Note: Because expr are evalutated left to right, that means temp handles are also
    /// acquired left to right... So release should happen right to left...
    /// At least that's what i have noticed in all of my unittests so far...
    release_temp_handle_if_active(rhs);
    release_temp_handle_if_active(lhs);
}

// Destructor is always called, even if pure virtual, so we need to explicitly define it.
inline SemanticSubsystem::~SemanticSubsystem() = default;
} // namespace alpha
#endif // SEMANTIC_SUBSYSTEM_HPP
