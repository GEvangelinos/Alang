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

CallBuilder::CallBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

CallBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services),
      method_call_draft_(std::string(), k_no_loc) {}

ConstBuilder::ConstBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

ConstBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

FunctionBuilder::FunctionBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

FunctionBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services),
      function_draft_(std::string(), k_no_loc) {}

TableAccessBuilder::TableAccessBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

TableAccessBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}
} // namespace alpha
