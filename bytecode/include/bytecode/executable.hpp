#ifndef EXECUTABLE_HPP
#define EXECUTABLE_HPP

#include "core/executable_types.h"
#include "core/string_cache,hpp.h"
#include "core/bytecode/vm_instruction.hpp"

namespace alpha::vm
{
struct Executable
{
    const StringCache str_literal_cache ;
    const StringCache progfunc_name_cache;
    const std::vector<vm::ProgFuncMetadata> progfuncs ;
    const std::vector<vm::Instruction> instructions ;
};

} // namespace alpha::vm

#endif // EXECUTABLE_HPP
