#include "core/string_span.hpp"

#include <cstring>

namespace alpha
{
char*
duplicate_to_cstring(const StringSpan ss)
{
    const std::size_t cstring_size = ss.size + 1; // +1 for null-byte '\0'
    char* const result = static_cast<char*>(std::malloc(cstring_size));
    if (!result) [[unlikely]]
        return nullptr;
    std::memcpy(result, ss.data, ss.size);
    result[ss.size] = '\0';
    return result;
}
} // namespace alpha
