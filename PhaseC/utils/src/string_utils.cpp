#include "utils/string_utils.hpp"

#include <algorithm>

namespace utils
{
std::string tolower_str(std::string str)
{
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](const unsigned char c) { return std::tolower(c); }
    );
    return str;
}

std::string toupper_str(std::string str)
{
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](const unsigned char c) { return std::toupper(c); }
    );
    return str;
}

std::string &lstrip(std::string &str)
{
    const auto it = std::find_if(
        str.begin(),
        str.end(),
        [](const unsigned char c) { return !std::isspace(c); }
    );
    str.erase(str.begin(), it);
    return str;
}

std::string &rstrip(std::string &str)
{
    const auto rit = std::find_if(
        str.rbegin(),
        str.rend(),
        [](const unsigned char c) { return !std::isspace(c); }
    );
    str.erase(rit.base(), str.end());
    return str;
}

std::string &strip(std::string &str)
{
    lstrip(str);
    rstrip(str);
    return str;
}

bool is_blank_str(const std::string &str)
{
    return str.empty() ||
           std::all_of(
               str.begin(), str.end(), [](const unsigned char ch) { return std::isspace(ch); }
           );
}
} // namespace utils
