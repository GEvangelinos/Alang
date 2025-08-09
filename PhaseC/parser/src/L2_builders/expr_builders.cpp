#include <L2_semantic_subsystems/expr_builders.hpp>

namespace alpha
{
AggregateBuilder::AggregateBuilder(const SemanticSystemServices &ss_services)
    :DISPATCH_TARGET(ss_services) {}

AggregateBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

AssignBuilder::AssignBuilder(
    AssignBuilder::Options &&options,
    const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(std::move(options), ss_services) {}

AssignBuilder::Restricted::Restricted(
    AssignBuilder::Options &&options,
    const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services),
      options_(options) {}

BasicBuilder::BasicBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

BasicBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

ConstBuilder::ConstBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

ConstBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}
} // namespace alpha
