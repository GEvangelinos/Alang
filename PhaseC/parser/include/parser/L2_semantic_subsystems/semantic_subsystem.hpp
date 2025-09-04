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

    void reset_temps_if_temp_operand(const Expr *unary);
    void reset_temps_if_temp_operand(const Expr *lhs, const Expr *rhs);

    virtual ~SemanticSubsystem() = 0;
};

inline void
SemanticSubsystem::reset_temps_if_temp_operand(const Expr *const unary)
{
    // Rvalue operands don't persist, so temp names can be safely reused.
    if (unary->has_temp_symbol())
        parse_ctx_->name_generator.reset_temps_at_checkpoint();
}

inline void
SemanticSubsystem::reset_temps_if_temp_operand(const Expr *const lhs, const Expr *const rhs)
{
    // Rvalue operands don't persist, so temp names can be safely reused.
    if (lhs->has_temp_symbol() || rhs->has_temp_symbol())
        parse_ctx_->name_generator.reset_temps_at_checkpoint();
}

// Destructor is always called, even if pure virtual, so we need to explicitly define it.
inline SemanticSubsystem::~SemanticSubsystem() = default;
} // namespace alpha
#endif // SEMANTIC_SUBSYSTEM_HPP
