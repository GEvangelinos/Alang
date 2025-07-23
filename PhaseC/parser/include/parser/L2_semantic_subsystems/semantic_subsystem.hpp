#ifndef SEMANTIC_SUBSYSTEM_HPP
#define SEMANTIC_SUBSYSTEM_HPP

#include <L1_driver/semantic_system_support.hpp>

namespace Alpha
{
class SemanticSubsystem
{
protected:
    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    DiagnosticReporter *const dr_ = nullptr;
    ExprMaker *const expr_maker_ = nullptr;
    ExprFolder *const expr_folder_ = nullptr;
    ExprSnitch *const expr_snitch_ = nullptr;
    QuadHandler *const quad_handler_ = nullptr;
    Backpatcher *const backpatcher_ = nullptr;
    SemanticSystemBridge *const ss_bridge_ = nullptr;

    explicit SemanticSubsystem(const SemanticSystemServices &ss_services)
        : parse_ctx_(REQUIRE_PTR(ss_services.parse_ctx)),
          symbol_table_(REQUIRE_PTR(ss_services.symbol_table)),
          dr_(REQUIRE_PTR(ss_services.dr)),
          expr_maker_(REQUIRE_PTR(ss_services.expr_maker)),
          expr_folder_(REQUIRE_PTR(ss_services.expr_folder)),
          expr_snitch_(REQUIRE_PTR(ss_services.expr_snitch)),
          quad_handler_(REQUIRE_PTR(ss_services.quad_handler)),
          backpatcher_(REQUIRE_PTR(ss_services.backpatcher)),
          ss_bridge_(REQUIRE_PTR(ss_services.ss_bridge)) {}

    virtual ~SemanticSubsystem() = 0;
};

inline SemanticSubsystem::~SemanticSubsystem() = default;
} // namespace Alpha
#endif // SEMANTIC_SUBSYSTEM_HPP
