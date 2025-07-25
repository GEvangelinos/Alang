#include <L2_semantic_subsystems/expr_builders.hpp>

namespace Alpha
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

BasicBuilder::BasicBuilder(Options &&options, const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(std::move(options), ss_services) {}

BasicBuilder::Restricted::Restricted(Options &&options, const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services), options_(options) {}

ConstBuilder::ConstBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

ConstBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}
} // namespace Alpha
