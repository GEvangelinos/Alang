#include "L2_semantic_subsystems/control_flow_managers.hpp"

namespace Alpha
{
const char *
LoopManager::to_string(const LoopKeyword lk)
{
    switch (lk)
    {
    case LoopKeyword::BREAK: return "break";
    case LoopKeyword::CONTINUE: return "continue";
    default: UNREACHABLE(FMT::format("Unknown `LoopKeyword`: int(lk) = ", static_cast<int>(lk)));
    }
}
} // namespace Alpha
