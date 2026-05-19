#include "support/string_tools.hpp"

#include <algorithm>

namespace alpha::support
{
std::string
tolower_str(std::string str)
{
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](const unsigned char c) { return std::tolower(c); }
    );
    return str;
}

std::string
toupper_str(std::string str)
{
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](const unsigned char c) { return std::toupper(c); }
    );
    return str;
}

std::string &
lstrip(std::string &str)
{
    const auto it = std::find_if(
        str.begin(),
        str.end(),
        [](const unsigned char c) { return !support::is_space(c); }
    );
    str.erase(str.begin(), it);
    return str;
}

std::string &
rstrip(std::string &str)
{
    const auto rit = std::find_if(
        str.rbegin(),
        str.rend(),
        [](const unsigned char c) { return !support::is_space(c); }
    );
    str.erase(rit.base(), str.end());
    return str;
}

std::string &
strip(std::string &str)
{
    lstrip(str);
    rstrip(str);
    return str;
}

bool
is_blank_str(const std::string &str)
{
    return str.empty() ||
           std::all_of(
               str.begin(), str.end(), [](const unsigned char ch) { return support::is_space(ch); }
           );
}

std::vector<std::string>
split_lines(const std::string &str)
{
    if (str.empty())
        return {};
    std::vector<std::string> result_lines;
    std::size_t start_pos = 0;
    while (true)
    {
        const auto newline_pos = str.find('\n', start_pos);
        result_lines.emplace_back(str.substr(start_pos, newline_pos - start_pos));
        if (newline_pos == std::string::npos)
            break;
        start_pos = newline_pos + 1;
    }
    return result_lines;
}
} // namespace alpha::support
