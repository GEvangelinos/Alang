#include "bytecode/abc_loader.hpp"

#include <concepts>
#include <list>
#include <vector>
#include <span>

#include "core/fixed_string.hpp"
#include "core/numeric_types.hpp"
#include "core/bytecode/abc_header.hpp"
#include "support/debug_tools.hpp"

namespace
{
template <FixedString msg>
void require(const auto op, const auto a, const auto b)
    requires(std::predicate<decltype(op), decltype(a), decltype(b)>)
{
    if (!op(a, b)) [[unlikely]]
        throw std::runtime_error(FMT::format(msg.data, a, b));
}
} // namespace

namespace alpha
{
[[nodiscard]] static abc::Header
load_header(const std::vector<u8>& buffer)
{
    constexpr auto GTE = std::greater_equal{};
    constexpr auto LT = std::less{};
    constexpr auto EQ = std::equal_to{};

    abc::Header header;
    require<"[ABC]: buffer size (0x{:X}) is smaller than mandatory header size (0x{:X})">
        (GTE, buffer.size(), sizeof(header));

    std::memcpy(&header, buffer.data(), sizeof(header));

    require<"[ABC] magic mismatch (expected 0x{:X}, got 0x{:X})">
        (EQ, abc::spec::k_magic_value, header.magic);
    require<"[ABC] filesize mismatch (expected 0x{:X}, got 0x{:X})">
        (EQ, header.file_size, buffer.size());
    require<"[ABC] Strings OOB: 0x{:X} >= 0x{:X}">(LT, header.offsets.strings, buffer.size());
    require<"[ABC] Lib-funcs OOB: 0x{:X} >= 0x{:X}">(LT, header.offsets.libfuncs, buffer.size());
    require<"[ABC] User-funcs OOB: 0x{:X} >= 0x{:X}">(LT, header.offsets.progfuncs, buffer.size());
    require<"[ABC] Code OOB: 0x{:X} >= 0x{:X}">(LT, header.offsets.code, buffer.size());

    return header;
}

struct StringCache
{
    struct StringInfo
    {
        u32 buffer_idx;
        u32 size;
    };

    std::vector<u8> str_buffer;
    std::vector<StringInfo> string_data;
};

[[nodiscard]] static StringCache
load_string_cache(const std::span<u8>& buffer)
{
    if (buffer.empty())
        return {};
    DMASSERT(buffer.begin() != buffer.end());

    StringCache str_cache;
    constexpr auto strlen_field_size = sizeof(abc::spec::StrLenT);
    const auto* const begin = buffer.data();       // Inclusive
    const auto* const end = begin + buffer.size(); // Exclusive

    const auto* ptr = begin;
    while (ptr < end)
    {
        // Get Strlen from buffer:
        abc::spec::StrLenT strlen = 0;
        std::memcpy(&strlen, ptr, strlen_field_size);
        ptr += strlen_field_size;

        // Copy string into continuous buffer and advance:
        const auto str_buffer_idx = str_cache.str_buffer.size();
        str_cache.string_data.emplace_back(str_buffer_idx, strlen);
        const auto last = ptr + strlen;
        str_cache.str_buffer.insert(str_cache.str_buffer.end(), ptr, last);
        ptr = last;
    }

    return str_cache;
}

int
ABC_Loader::load(const std::vector<u8>& byte_buffer)
{
    const abc::Header header = load_header(byte_buffer);

    const auto str_begin = header.offsets.strings
    const auto str_cache = load_string_cache(byte_buffer);
    std::cout << std::endl;
}
} // namespace alpha
