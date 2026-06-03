#ifndef VM_PROGRAM_HPP
#define VM_PROGRAM_HPP

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "core/numeric_types.hpp"
#include "core/string_span.hpp"
#include "vm_instruction.hpp"
#include "support/bidirectional_registry.hpp"

namespace alpha::vm
{
struct Program
{
    using StringID = u32;
    using ProgFuncID = u32;

    struct ProgFunc
    {
        StringID name_str_id;
        CodeAddress address;
        u32 local_size;
    };

    std::vector<Instruction> instructions;
    std::vector<ProgFunc> progfuncs;

    support::BidirectionalRegistry<StringSpan, StringID> str_literal_registry;
    support::BidirectionalRegistry<StringSpan, ProgFuncID> progfunc_name_registry;
    std::unordered_set<StringSpan> libfunc_name_table;
};
} // namespace alpha::vm
#endif //VM_PROGRAM_HPP
