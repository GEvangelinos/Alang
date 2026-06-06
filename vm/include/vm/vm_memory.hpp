#ifndef VM_MEMORY_HPP
#define VM_MEMORY_HPP

#include <unordered_map>
#include "core/code_address.hpp"
#include "core/machine_types.hpp"
#include "core/numeric_types.hpp"
#include "support/debug_tools.hpp"
#include "support/misc_tools.hpp"

namespace alpha::vm
{
struct Table;

struct Memcell
{
    #define MEMCELL_TYPE(X)\
        X(UNDEF) /* UNDEF SHOULD always be zero, (as we memset stack to 0, at initialization) */ \
        X(INT)\
        X(FLOAT)\
        X(STRING)\
        X(BOOL)\
        X(TABLE)\
        X(PROGFUNC)\
        X(LIBFUNC)\
        X(NIL)


    enum class Type : u8
    {
        #define MEMCELL_TYPE_TO_ENUM(TYPE) TYPE,
        MEMCELL_TYPE(MEMCELL_TYPE_TO_ENUM)
        #undef MEMCELL_TYPE_TO_ENUM
    };

    Type type;

    union
    {
        AlphaInt int_value;
        AlphaFloat float_value;
        const char* str_value;
        bool bool_value;
        Table* table_value;
        u32 progfunc_index;
        const char* libfunc_name;
        // TODO "For libfuncs name to int index translation would be better, so we dont have to hash libfuncs every time..."
    } data;

    static_assert(sizeof(data) == 8);

    void clear();
    void clear_string();
    void clear_table();

    [[nodiscard]] bool is_arithmetic() const noexcept;

    [[nodiscard]] std::string to_string() const noexcept;
    [[nodiscard]] bool to_bool() const noexcept;
};

[[nodiscard]] inline const char *
to_string(const vm::Memcell::Type memcell_type)
{
    switch (memcell_type)
    {
    #define MEMCELL_TYPE_TO_STRING(TYPE) case vm::Memcell::Type::TYPE: return #TYPE;
    MEMCELL_TYPE(MEMCELL_TYPE_TO_STRING)
    #undef MEMCELL_TYPE_TO_STRING
    default: DMASSERT(false && "Unknown vm::Memcell::Type");
    }
    std::abort();
}
#undef  MEMCELL_TYPE

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
        // clear_string();
        DMASSERT(false);
        break;
    case Type::TABLE:
        // clear_table();
        DMASSERT(false);
        break;
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
    default: DMASSERT(false);
    }
    return "__INTERNAL_ERROR__: 2026.05.31.22:07";
}

inline bool
Memcell::to_bool() const noexcept
{
    switch (type)
    {
    case Type::UNDEF:
    case Type::NIL: return false;
    case Type::INT: return data.int_value != 0;
    case Type::FLOAT: return data.float_value != 0.0;
    case Type::STRING: return DEBUG_REQUIRE_PTR(data.str_value)[0] != '\0';
    case Type::BOOL: return data.bool_value;
    case Type::TABLE:
    case Type::PROGFUNC:
    case Type::LIBFUNC: return true;
    default: DMASSERT(false && "Unknown Memcell::Type");
    }
    std::abort();
}

inline bool
Memcell::is_arithmetic() const noexcept { return type == Type::INT || type == Type::FLOAT; }
} // namespace alpha::vm
#endif //VM_MEMORY_HPP
