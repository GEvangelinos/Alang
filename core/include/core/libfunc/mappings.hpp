#ifndef MAPPINGS_HPP
#define MAPPINGS_HPP

#include <array>
#include <optional>
#include "id.hpp"

namespace alpha::vm
{
static constexpr std::array k_library_functions_names{
    // -1 is required cause sizeof also counts '\0'
    #define X(id, name) StringSpan{name, sizeof(name) - 1},
    #include "libfuncs.def"
    #undef  X
};

[[nodiscard]] inline std::optional<LibFuncId>
get_libfunc_id(const StringSpan libfunc_name)
{
    for (decltype(k_library_functions_names)::size_type i = 0; i < k_library_functions_names.size();
         ++i)
    {
        const StringSpan ith_name = k_library_functions_names[i];
        if (libfunc_name == ith_name)
            return static_cast<LibFuncId>(i);
    }
    return std::nullopt;
}

[[nodiscard]] inline std::optional<StringSpan>
get_libfunc_name(LibFuncId libfunc_id)
{
    const auto libfunc_idx = static_cast<std::underlying_type_t<LibFuncId>>(libfunc_id);
    if (libfunc_idx < k_library_functions_names.size())
        return k_library_functions_names[libfunc_idx];
    return std::nullopt;
}
} // namespace alpha::vm
#endif // MAPPINGS_HPP
