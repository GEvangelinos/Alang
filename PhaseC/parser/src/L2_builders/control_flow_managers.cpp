#include "L2_semantic_subsystems/control_flow_managers.hpp"

namespace Alpha
{
LoopManager::LoopManager(const SemanticSystemServices &ss_services)
    :DISPATCH_TARGET(ss_services) {}

LoopManager::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

const char *
LoopManager::Restricted::to_string(const LoopKeyword lk)
{
    switch (lk)
    {
    case LoopKeyword::BREAK: return "break";
    case LoopKeyword::CONTINUE: return "continue";
    default: UNREACHABLE(FMT::format("Unknown `LoopKeyword`: int(lk) = ", static_cast<int>(lk)));
    }
}
} // namespace Alpha
