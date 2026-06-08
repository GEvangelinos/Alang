#include "vm/vm_memory.hpp"
#include "support/format_adapter.hpp"

namespace alpha::vm
{
std::string
Table::to_string(const u32 call_depth) const
{
    std::string result;

    const auto format_hash_table = [call_depth]<typename KeyType>(
        const HashTable<KeyType>& hash_table) -> std::string
    {
        constexpr const char* key_delimiter = std::is_same_v<KeyType, std::string> ? "\"" : "";

        const char* const nl = hash_table.size() > 1 ? "\n" : "";

        std::string result = "[";
        result += nl;
        for (auto it = hash_table.begin(); it != hash_table.end(); ++it)
        {
            std::string formatted_key;
            if constexpr (std::is_enum_v<KeyType>)
                formatted_key
                    = FMT::format("{}", static_cast<std::underlying_type_t<KeyType>>(it->first));
            else if constexpr (std::is_pointer_v<KeyType>)
                formatted_key = FMT::format("{}", static_cast<const void*>(it->first));
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
