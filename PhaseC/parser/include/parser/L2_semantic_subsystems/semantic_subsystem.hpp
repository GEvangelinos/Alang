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
        : parse_ctx_(REQUIRE_PTR(ss_services.parse_ctx)),
          symbol_table_(REQUIRE_PTR(ss_services.symbol_table)),
          dr_(REQUIRE_PTR(ss_services.dr)),
          expr_maker_(REQUIRE_PTR(ss_services.expr_maker)),
          expr_optimizer_(REQUIRE_PTR(ss_services.expr_optimizer)),
          quad_handler_(REQUIRE_PTR(ss_services.quad_handler)),
          ss_bridge_(REQUIRE_PTR(ss_services.ss_bridge)) {}

    virtual ~SemanticSubsystem() = 0;
};

// Destructor is always called, even if pure virtual, so we need to explicitly define it.
inline SemanticSubsystem::~SemanticSubsystem() = default;
} // namespace alpha
#endif // SEMANTIC_SUBSYSTEM_HPP
