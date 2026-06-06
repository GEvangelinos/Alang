#ifndef EXECUTABLE_TYPES_H
#define EXECUTABLE_TYPES_H

#include "code_address.hpp"
#include "numeric_types.hpp"

namespace alpha::vm
{

using StringID = u32;
using ProgFuncID = u32;

struct ProgFunc
{
    StringID name_str_id;
    CodeAddress address;
    u32 local_size;
};
} // namespace alpha::vm
#endif //EXECUTABLE_TYPES_H
