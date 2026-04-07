#ifndef VM_PROGRAM_HPP
#define VM_PROGRAM_HPP

#include <unordered_map>
#include <vector>
#include "core/numeric_types.hpp"
#include "core/string_span.hpp"
#include "vm_instructions.hpp"

namespace alpha::vm
{
struct Program
{
    using StringID = u32;
    using LibfuncID = u32;

    struct UserFunc
    {
        StringSpan id;
        u32 address;
        u32 local_size;
    };

    std::vector<Instruction> code;
    std::vector<UserFunc> userfuncs;
    std::unordered_map<StringSpan, StringID> string_literal_table;
    std::unordered_map<StringSpan, LibfuncID> libfunc_name_table;
};
} // namespace alpha::vm
#endif //VM_PROGRAM_HPP
