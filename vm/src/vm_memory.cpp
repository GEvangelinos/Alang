#include "vm/vm_memory.hpp"

#include <vector>

#include "support/format_adapter.hpp"


namespace alpha::vm
{
std::string
Memcell::to_string(const bool include_str_quotes) const
{
    switch (type)
    {
    case Type::UNDEF: return "undefined";
    case Type::INT: return FMT::to_string(data.int_value);
    case Type::FLOAT:
        return data.float_value == static_cast<AlphaInt>(data.float_value)
               ? FMT::to_string(data.float_value) + ".0"
               : FMT::to_string(data.float_value);

    case Type::STRING:
    {
        const char *const delimeter = include_str_quotes ? "\"" : "";
        return FMT::format("{}{}{}", delimeter, std::string(data.str_value), delimeter);
    }
    case Type::BOOL: return std::string(data.bool_value ? "true" : "false");
    case Type::TABLE: return data.table_value->to_string();
    case Type::PROGFUNC: return FMT::format("(program_func: {})", data.progfunc_index);
    case Type::LIBFUNC:
    {
        const std::optional<StringSpan> libname = get_libfunc_name(data.libfunc_id);
        DMASSERT(libname.has_value() && "How did it become a type LIBFUNC then.. ?");
        return FMT::format("{}()", libname->data);
    }
    case Type::NIL: return "nil";
    default: DMASSERT(false);
    }
    return "__INTERNAL_ERROR__: 2026.05.31.22:07";
}

std::string
Table::to_string() const
{
    std::string result = "[";

    const auto format_hash_table = [&result]<typename KeyType>(const HashTable<KeyType> &hash_table)
    {
        constexpr auto str_delimiter = std::is_same_v<KeyType, std::string> ? "\"" : "";

        using PairType = typename HashTable<KeyType>::value_type;

        std::vector<const PairType *> ordered;
        ordered.reserve(hash_table.size());
        for (const auto &elem: hash_table)
            ordered.push_back(&elem);

        std::sort(
            ordered.begin(), ordered.end(),
            [](const PairType *const a, const PairType *const b) { return a->first < b->first; }
        );

        for (const PairType *const pair: ordered)
        {
            const auto &[key,val] = *pair;
            FMT::format_to(
                std::back_inserter(result),
                "{{{0}{1}{0} : {2}}}, ",
                str_delimiter, key, val.to_string()
            );
        }
    };

    format_hash_table(int_indexed);
    format_hash_table(float_indexed);
    format_hash_table(str_indexed);
    format_hash_table(bool_indexed);
    format_hash_table(progfunc_indexed);
    format_hash_table(libfunc_indexed);

    constexpr std::string_view suffix{", "};
    if (result.ends_with(suffix))
        result.resize(result.size() - suffix.size());

    result += "]";
    return result;
}

std::string
Table::inspect(const u32 call_depth) const
{
    std::string result;

    const auto format_hash_table = [call_depth]<typename KeyType>(
        const HashTable<KeyType> &hash_table) -> std::string
    {
        constexpr const char *key_delimiter = std::is_same_v<KeyType, std::string> ? "\"" : "";

        const char *const nl = hash_table.size() > 1 ? "\n" : "";

        std::string result = "[";
        result += nl;
        for (auto it = hash_table.begin(); it != hash_table.end(); ++it)
        {
            std::string formatted_key;
            if constexpr (std::is_same_v<KeyType, LibFuncId>)
            {
                const std::optional<StringSpan> libname = get_libfunc_name(it->first);
                DMASSERT(libname.has_value() && "How did it become a type LIBFUNC then.. ?");
                formatted_key = FMT::format("{}()", libname->data);
            }
            else if constexpr (std::is_pointer_v<KeyType>)
                formatted_key = FMT::format("{}", static_cast<const void *>(it->first));
            else
                formatted_key = FMT::format("{}", it->first);

            result.append(FMT::format(
                "{0}{{{1}{2}{3} : {4}}}{5}",
                hash_table.size() > 1
                ? std::string(call_depth ? call_depth * 23 : 23, ' ')
                : std::string(),
                key_delimiter,
                formatted_key,
                key_delimiter,
                it->second.to_string(true),
                nl
            ));
        }
        result.append(
            hash_table.size() > 1
            ? std::string(call_depth ? call_depth * 4 : 4, ' ')
            : std::string()
        );
        result.append("]");
        return result;
    };

    result = FMT::format(
        R"(
[
    __ref_count__: {}
    __bool__     : {}
    __int__      : {}
    __float__    : {}
    __string__   : {}
    __progfunc__ : {}
    __libfunc__  : {}
    __table__    : {}
]
)",
        ref_counter,
        format_hash_table(bool_indexed),
        format_hash_table(int_indexed),
        format_hash_table(float_indexed),
        format_hash_table(str_indexed),
        format_hash_table(progfunc_indexed),
        format_hash_table(libfunc_indexed),
        format_hash_table(table_indexed)
    );
    return result;
}
} // namespace alpha::vm
