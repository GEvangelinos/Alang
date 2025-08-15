#include "L2_semantic_subsystems/control_flow_managers.hpp"

namespace alpha
{
ControlFlowManager::ControlFlowManager(const SemanticSystemServices &ss_services)
    :DISPATCH_TARGET(ss_services) {}

ControlFlowManager::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}
} // namespace alpha
