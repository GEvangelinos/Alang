#ifndef VM_MEMCELL_HPP
#define VM_MEMCELL_HPP

#include "core/code_address.hpp"
#include "core/machine_types.hpp"
#include "core/numeric_types.hpp"
#include "support/debug_tools.hpp"

namespace alpha::vm
{
struct Memcell
{
    enum class Type : u8
    {
        UNDEF = 0, // UNDEF SHOULD always be zero, (as we memset stack to 0, at initialization)
        INT,
        FLOAT,
        STRING,
        BOOL,
        TABLE,
        PROGFUNC,
        LIBFUNC,
        NIL,
    };

    Type type;

    union
    {
        AlphaInt int_value;
        AlphaFloat float_value;
        const char* str_value;
        bool bool_value;
        void* table_value;
        CodeAddress progfunc_address;
        const char* libfunc_name;
        #warning "For libfuncs name to int index translation would be better, so we dont have to hash libfuncs every time..."
    } data;

    static_assert(sizeof(data) == 8);

    void clear();
    void clear_string();
    void clear_table();
};

inline void
Memcell::clear()
{
    if (this->type != Type::UNDEF)
        return;
    switch (this->type)
    {
    case Type::BOOL:
    case Type::FLOAT:
    case Type::INT:
    case Type::UNDEF:
    case Type::NIL:
    case Type::PROGFUNC:
    case Type::LIBFUNC:
        this->type = Type::UNDEF;
        return;
    case Type::STRING:
        clear_string();
        DMASSERT(false);
    case Type::TABLE:
        clear_table();
        DMASSERT(false);
    default: DMASSERT(false && "Unknown Memcell::Type");
    }
}
} // namespace alpha::vm

#endif // VM_MEMCELL_HPP
