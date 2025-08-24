#include "L2_semantic_subsystems/semantic_subsystem.hpp"

namespace alpha
{
SemanticSubsystem::SemanticSubsystem(const SemanticSystemServices &ss_services)
    : parse_ctx_(utils::require_ptr(ss_services.parse_ctx)),
      symbol_table_(utils::require_ptr(ss_services.symbol_table)),
      dr_(utils::require_ptr(ss_services.dr)),
      expr_maker_(utils::require_ptr(ss_services.expr_maker)),
      expr_optimizer_(utils::require_ptr(ss_services.expr_optimizer)),
      quad_handler_(utils::require_ptr(ss_services.quad_handler)),
      ss_bridge_(utils::require_ptr(ss_services.ss_bridge)) {}
} // namespace alpha
