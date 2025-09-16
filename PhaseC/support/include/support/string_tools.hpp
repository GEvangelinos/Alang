#ifndef SUPPORT_STRING_TOOLS_HPP
#define SUPPORT_STRING_TOOLS_HPP

#include <cstring>
#include <string>
#include "support/debug_tools.hpp"

namespace alpha::support
{
[[nodiscard]] std::string tolower_str(std::string str);
[[nodiscard]] std::string toupper_str(std::string str);
std::string &lstrip(std::string &str);
std::string &rstrip(std::string &str);
std::string &strip(std::string &str);
bool is_blank_str(const std::string &str);

[[nodiscard]] inline char *cstrdup(const char *const src)
{
    DEBUG_SMART_ASSERT(!!src);
    if (!src)
        return nullptr;

    const auto src_size = std::strlen(src) + 1; // +1 for NULL-byte
    char *dest = new char[src_size];
    std::memcpy(dest, src, src_size);
    return dest;
}
} // namespace alpha::support
#endif // SUPPORT_STRING_TOOLS_HPP
