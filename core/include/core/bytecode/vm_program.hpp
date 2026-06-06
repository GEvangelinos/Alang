#ifndef VM_PROGRAM_HPP
#define VM_PROGRAM_HPP

#include <vector>
#include "core/string_span.hpp"
#include "vm_instruction.hpp"
#include "core/executable_types.h"
#include "support/bidirectional_registry.hpp"

namespace alpha::vm
{
struct Program
{
    std::vector<Instruction> instructions;
    std::vector<ProgFuncMetadata> progfuncs;

    support::BidirectionalRegistry<StringSpan, StringID> str_literal_registry;
    support::BidirectionalRegistry<StringSpan, ProgFuncID> progfunc_name_registry;
    support::BidirectionalRegistry<CodeAddress::UnderlyingType, u32> progfunc_registry;
};
} // namespace alpha::vm
#endif //VM_PROGRAM_HPP
