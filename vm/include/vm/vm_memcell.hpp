#ifndef VM_MEMCELL_HPP
#define VM_MEMCELL_HPP

#include "core/machine_types.hpp"
#include "core/numeric_types.hpp"

namespace alpha::vm
{
struct Memcell
{
    enum class Type : u8
    {
        INT,
        FLOAT,
        STRING,
        BOOL,
        TABLE,
        PROGFUNC,
        LIBFUNC,
        NIL,
        UNDEF,
    };

    Type type;

    union
    {
        AlphaInt int_value;
        AlphaFloat float_value;
        const char* str_value;
        bool bool_value;
        void* table_value;
        u32 progfunc_idx;
        const char* libfunc_name;
        #warning "For libfuncs name to int index translation would be better, so we dont have to hash libfuncs every time..."
    } data;
};
} // namespace alpha::vm

#endif // VM_MEMCELL_HPP
