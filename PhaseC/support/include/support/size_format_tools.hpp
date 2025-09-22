#ifndef SUPPORT_SIZE_FORMAT_TOOLS_HPP
#define SUPPORT_SIZE_FORMAT_TOOLS_HPP

#include <cstddef>
#include <string>

namespace alpha::support
{
struct DataSize
{
    const double size;
    const char *unit;

    [[nodiscard]] std::string to_string(unsigned precision = 2);
};

[[nodiscard]] DataSize format_bitsize(std::size_t bytesize) noexcept;
[[nodiscard]] DataSize format_bytesize(std::size_t bytesize) noexcept;
} // namespace alpha::support

#endif // SUPPORT_SIZE_FORMAT_TOOLS_HPP
