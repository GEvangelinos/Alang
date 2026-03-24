#ifndef VM_PROGRAM_HPP
#define VM_PROGRAM_HPP

#include <vector>
#include "core/numeric_types.hpp"
#include "core/string_span.hpp"
#include "vm_instructions.hpp"

namespace alpha::vm
{
struct Program
{
    struct UserFunc
    {
        u32 address;
        u32 local_size;
        StringSpan id;
    };

    std::vector<Instruction> code;
    std::vector<UserFunc> userfunc_table;
    std::vector<StringSpan> string_literal_pool;
    std::vector<StringSpan> libfunc_name_pool;
};
} // namespace alpha::vm
#endif //VM_PROGRAM_HPP
