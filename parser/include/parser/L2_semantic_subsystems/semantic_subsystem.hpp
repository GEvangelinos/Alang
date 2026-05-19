#ifndef SEMANTIC_SUBSYSTEM_HPP
#define SEMANTIC_SUBSYSTEM_HPP

#include <concepts>
#include <type_traits>
#include <L1_driver/semantic_system_support.hpp>

#include "core/quad_yielder.hpp"

namespace alpha
{
class SemanticSubsystem : private Immobile
{
protected:
    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    DiagnosticReporter *const dr_ = nullptr;
    ExprMaker *const expr_maker_ = nullptr;
    QuadHandler *const quad_handler_ = nullptr;
    QuadYielder *const quad_yielder_ = nullptr;
    ExprNormalizer *const expr_normalizer_ = nullptr;
    ExprOptimizer *const expr_optimizer_ = nullptr;

    explicit SemanticSubsystem(const SemanticSystemServices &ss_services);

    virtual ~SemanticSubsystem() = 0;
};
// Destructor is always called, even if pure virtual, so we need to explicitly define it.
inline
SemanticSubsystem::~SemanticSubsystem() = default;
} // namespace alpha
#endif // SEMANTIC_SUBSYSTEM_HPP
