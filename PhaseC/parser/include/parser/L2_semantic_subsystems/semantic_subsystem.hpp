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
    if (unary->has_active_tempvar())
    {
        const auto temp_slot = static_cast<const ExprWVarSymbol *>(unary)->var_symbol->temp_slot_id;
        DEBUG_SMART_ASSERT(temp_slot.has_value());
        parse_ctx_->temp_ctx_handler.release_temp_slot(*temp_slot);
    }
}

inline void
SemanticSubsystem::reset_temps_if_temp_operand(const Expr *const lhs, const Expr *const rhs)
{
    reset_temps_if_temp_operand(lhs);
    reset_temps_if_temp_operand(rhs);
}

// Destructor is always called, even if pure virtual, so we need to explicitly define it.
inline SemanticSubsystem::~SemanticSubsystem() = default;
} // namespace alpha
#endif // SEMANTIC_SUBSYSTEM_HPP
