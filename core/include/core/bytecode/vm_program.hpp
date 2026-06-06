#ifndef VM_PROGRAM_HPP
#define VM_PROGRAM_HPP

#include <unordered_set>
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
    std::vector<ProgFunc> progfuncs;

    support::BidirectionalRegistry<StringSpan, StringID> str_literal_registry;
    support::BidirectionalRegistry<StringSpan, ProgFuncID> progfunc_name_registry;
    std::unordered_set<StringSpan> libfunc_name_table;
};
} // namespace alpha::vm
#endif //VM_PROGRAM_HPP
