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
    ExprOptimizer*const expr_optimizer_ = nullptr;
    QuadHandler *const quad_handler_ = nullptr;
    SemanticSystemBridge *const ss_bridge_ = nullptr;

    explicit SemanticSubsystem(const SemanticSystemServices &ss_services)
        : parse_ctx_(utils::require_ptr(ss_services.parse_ctx)),
          symbol_table_(utils::require_ptr(ss_services.symbol_table)),
          dr_(utils::require_ptr(ss_services.dr)),
          expr_maker_(utils::require_ptr(ss_services.expr_maker)),
          expr_optimizer_(utils::require_ptr(ss_services.expr_optimizer)),
          quad_handler_(utils::require_ptr(ss_services.quad_handler)),
          ss_bridge_(utils::require_ptr(ss_services.ss_bridge)) {}

    virtual ~SemanticSubsystem() = 0;
};

// Destructor is always called, even if pure virtual, so we need to explicitly define it.
inline SemanticSubsystem::~SemanticSubsystem() = default;
} // namespace alpha
#endif // SEMANTIC_SUBSYSTEM_HPP
