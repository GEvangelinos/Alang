#ifndef VM_PROGRAM_HPP
#define VM_PROGRAM_HPP

#include <unordered_map>
#include <vector>
#include "core/numeric_types.hpp"
#include "core/string_span.hpp"
#include "vm_instruction.hpp"

namespace alpha::vm
{
struct Program
{
    using StringID = u32;
    using LibfuncID = u32;

    struct ProgFunc
    {
        StringSpan id;
        LabelID address;
        u32 local_size;
    };

    struct
    {
        u32 total_string_size = 0;
    } metadata;

    std::vector<Instruction> instructions;
    std::vector<ProgFunc> progfuncs;
    std::unordered_map<StringSpan, StringID> str_literal_table;
    std::unordered_map<StringSpan, LibfuncID> libfunc_name_table;
};
} // namespace alpha::vm
#endif //VM_PROGRAM_HPP
