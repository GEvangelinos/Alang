#include "support/size_format_tools.hpp"
#include "support/format_adapter.hpp"


static constexpr auto BITS_PER_BYTE = 8;
static constexpr auto KB = 1ULL << 10;
static constexpr auto MB = 1ULL << 20;
static constexpr auto GB = 1ULL << 30;
static constexpr auto TB = 1ULL << 40;
static constexpr auto PB = 1ULL << 50;
static constexpr auto EB = 1ULL << 60;

namespace alpha::support
{
std::string DataSize::to_string(const unsigned precision = 2) // Default precision is 2 floats
{
    return FMT::format("{0:.{1}f} {2}", this->size, precision, this->unit);
}

DataSize format_bitsize(const size_t bytesize) noexcept
{
    const size_t bitsize = BITS_PER_BYTE * bytesize;
    if (bitsize < KB) return {.size = static_cast<double>(bitsize), .unit = "b"};
    if (bitsize < MB) return {.size = bitsize / static_cast<double>(KB), .unit = "Kb"};
    if (bitsize < GB) return {.size = bitsize / static_cast<double>(MB), .unit = "Mb"};
    if (bitsize < TB) return {.size = bitsize / static_cast<double>(GB), .unit = "Gb"};
    if (bitsize < PB) return {.size = bitsize / static_cast<double>(TB), .unit = "Tb"};
    if (bitsize < EB) return {.size = bitsize / static_cast<double>(PB), .unit = "Pb"};
    return {.size = bitsize / static_cast<double>(EB), .unit = "Eb"};
}

DataSize format_bytesize(const size_t bytesize) noexcept
{
    if (bytesize < KB) return {.size = static_cast<double>(bytesize), .unit = "B"};
    if (bytesize < MB) return {.size = bytesize / static_cast<double>(KB), .unit = "KB"};
    if (bytesize < GB) return {.size = bytesize / static_cast<double>(MB), .unit = "MB"};
    if (bytesize < TB) return {.size = bytesize / static_cast<double>(GB), .unit = "GB"};
    if (bytesize < PB) return {.size = bytesize / static_cast<double>(TB), .unit = "TB"};
    if (bytesize < EB) return {.size = bytesize / static_cast<double>(PB), .unit = "PB"};
    return {.size = bytesize / static_cast<double>(EB), .unit = "EB"};
}
} // namespace alpha::support
