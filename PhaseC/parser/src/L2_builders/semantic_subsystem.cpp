#include "L2_semantic_subsystems/semantic_subsystem.hpp"
#include "../../include/parser/L2_semantic_subsystems/core/quad_handler.hpp"

namespace alpha
{
SemanticSubsystem::SemanticSubsystem(const SemanticSystemServices &ss_services)
    : parse_ctx_(support::require_ptr(ss_services.parse_ctx)),
      symbol_table_(support::require_ptr(ss_services.symbol_table)),
      dr_(support::require_ptr(ss_services.dr)),
      expr_maker_(support::require_ptr(ss_services.expr_maker)),
      quad_handler_(support::require_ptr(ss_services.quad_handler)),
      quad_yielder_(support::require_ptr(ss_services.quad_yielder)),
      expr_normalizer_(support::require_ptr(ss_services.expr_normalizer)),
      expr_optimizer_(support::require_ptr(ss_services.expr_optimizer)) {}
} // namespace alpha
