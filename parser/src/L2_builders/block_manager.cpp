#include <L2_semantic_subsystems/block_manager.hpp>

namespace alpha
{
BlockManager::BlockManager(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

BlockManager::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}
} // namespace alpha
