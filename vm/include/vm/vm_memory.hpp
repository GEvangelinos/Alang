#ifndef VM_MEMORY_HPP
#define VM_MEMORY_HPP

#include <ranges>
#include <unordered_map>
#include "core/code_address.hpp"
#include "core/machine_types.hpp"
#include "core/numeric_types.hpp"
#include "core/libfunc/id.hpp"
#include "core/libfunc/mappings.hpp"
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
        // TODO: make a separate allocation and deallocation function that accepts both const char * and StringSpans (overloaded) that uses the new[] allocator (not std::malloc())
        AlphaInt int_value;
        AlphaFloat float_value;
        const char *str_value; // Malloc'd (dont use new/ new[])
        bool bool_value;
        Table *table_value;
        u32 progfunc_index;
        vm::LibFuncId libfunc_id;
        // TODO "For libfuncs name to int index translation would be better, so we dont have to hash libfuncs every time..."
    } data;

    static_assert(sizeof(data) == 8);

    void clear();

    [[nodiscard]] bool is_arithmetic() const noexcept;

    [[nodiscard]] std::string to_string(bool include_str_quotes = false) const;
    [[nodiscard]] bool to_bool() const noexcept;


    [[nodiscard]] friend const char *to_string(const vm::Memcell::Type memcell_type) noexcept
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
};

// TODO: use a flat hashmap, currently there is a lot of pointer chasing.
template<typename KeyType>
using HashTable = std::unordered_map<KeyType, vm::Memcell>;

struct Table
{
    #ifdef DEBUG_MODE

private:
    static i64 serial_number_counter;

public:
    const i64 serial_number = -1;

    Table() : serial_number(serial_number_counter++) {}


    #endif // DEBUG_MODE

    HashTable<std::string> str_indexed;
    HashTable<AlphaInt> int_indexed;
    HashTable<AlphaFloat> float_indexed;
    HashTable<bool> bool_indexed;
    HashTable<u32> progfunc_indexed;
    HashTable<vm::LibFuncId> libfunc_indexed;
    HashTable<Table *> table_indexed;

    u32 ref_counter = 0;

    void increase_ref() noexcept { ++ref_counter; }

    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] std::string inspect(u32 call_depth = 0) const;

    [[nodiscard]] u64 size() const noexcept;
};

inline u64
Table::size() const noexcept
{
    return str_indexed.size() +
           int_indexed.size() +
           float_indexed.size() +
           bool_indexed.size() +
           progfunc_indexed.size() +
           libfunc_indexed.size() +
           table_indexed.size();
}

inline void
decrease_ref(Table *t)
{
    const auto clear_values = [](const auto memcells)
    {
        for (Memcell &cell: memcells)
            cell.clear();
    };

    DMASSERT(!!t);
    DMASSERT(t->ref_counter > 0);
    if (!--t->ref_counter)
    {
        clear_values(t->str_indexed | std::views::values);
        clear_values(t->int_indexed | std::views::values);
        clear_values(t->float_indexed | std::views::values);
        clear_values(t->bool_indexed | std::views::values);
        clear_values(t->progfunc_indexed | std::views::values);
        clear_values(t->libfunc_indexed | std::views::values);
        clear_values(t->table_indexed | std::views::values);

        for (Table *key: t->table_indexed | std::views::keys)
            decrease_ref(key);
        delete t;
    }
}

inline void
Memcell::clear()
{
    switch (type)
    {
    case Type::UNDEF:
        return;
    case Type::BOOL:
    case Type::FLOAT:
    case Type::INT:
    case Type::NIL:
    case Type::PROGFUNC:
    case Type::LIBFUNC:
        type = Type::UNDEF;
        return;
    case Type::STRING:
        type = Type::UNDEF;
        std::free(const_cast<char *>(data.str_value));
        break;
    case Type::TABLE:
        decrease_ref(data.table_value);
        type = Type::UNDEF;
        break;
    default: DMASSERT(false && "Unknown Memcell::Type");
    }
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
