#ifndef VM_MEMORY_HPP
#define VM_MEMORY_HPP

#include <unordered_map>
#include "core/code_address.hpp"
#include "core/machine_types.hpp"
#include "core/numeric_types.hpp"
#include "support/debug_tools.hpp"

namespace alpha::vm
{
struct Table;

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
        Table* table_value;
        CodeAddress::UnderlyingType progfunc_address;
        const char* libfunc_name;
        // TODO "For libfuncs name to int index translation would be better, so we dont have to hash libfuncs every time..."
    } data;

    static_assert(sizeof(data) == 8);

    void clear();
    void clear_string();
    void clear_table();

    [[nodiscard]] std::string to_string() const noexcept;
};

struct HashTable
{
    // TODO: use a flat hashmap, currently there is a lot of pointer chasing.
    // std::unordered_map<vm::Memcell, vm::Memcell> data;
};

struct Table
{
    HashTable str_indexed;
    HashTable int_indexed;
    HashTable float_indexed;
    HashTable progfuncs;
    HashTable libfuncs;

    u32 ref_counter;

    void increase_ref() noexcept { ++ref_counter; }
    void decrease_ref() noexcept { --ref_counter; }
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

inline std::string
Memcell::to_string() const noexcept
{
    switch (type)
    {
    case Type::UNDEF: return "undefined";
    case Type::INT: return FMT::to_string(data.int_value);
    case Type::FLOAT: return FMT::to_string(data.float_value);
    case Type::STRING: return std::string(data.str_value);
    case Type::BOOL: return std::string(data.bool_value ? "true" : "false");
    case Type::TABLE: DMASSERT(false);
        break;
    case Type::PROGFUNC: DMASSERT(false);
        break;
    case Type::LIBFUNC: return FMT::format("{}()", data.libfunc_name);
    case Type::NIL: return "nil";
    }
}
} // namespace alpha::vm
#endif //VM_MEMORY_HPP
