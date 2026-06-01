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
        throw std::runtime_error("[ABC]: " + FMT::format(msg.data, a, b));
}
} // namespace

namespace alpha
{
[[nodiscard]] static abc::Header
load_header(const std::vector<u8>& buffer)
{
    abc::Header header;
    require<"buffer size (0x{:X}) is smaller than mandatory header size (0x{:X})">
        (std::greater_equal{}, buffer.size(), sizeof(header));

    std::memcpy(&header, buffer.data(), sizeof(header));

    require<"magic mismatch (expected 0x{:X}, got 0x{:X})">
        (std::equal_to{}, abc::spec::k_magic_value, header.magic);
    require<"filesize mismatch (expected 0x{:X}, got 0x{:X})">
        (std::equal_to{}, header.file_size, buffer.size());
    require<"strings expected: 0x{:X} < 0x{:X}">
        (std::less{}, header.sections.strings.lut.offset, buffer.size());
    require<"lib-funcs expected: 0x{:X} < 0x{:X}">
        (std::less{}, header.sections.libfuncs.lut.offset, buffer.size());
    require<"user-funcs expected: 0x{:X} < 0x{:X}">
        (std::less{}, header.sections.progfuncs.lut.offset, buffer.size());
    require<"instructions expected: 0x{:X} < 0x{:X}">
        (std::less{}, header.sections.instructions.offset, buffer.size());

    return header;
}

// [[nodiscard]] static StringCache
// load_string_cache(const std::span<u8>& buffer)
// {
//     if (buffer.empty())
//         return {};
//     DMASSERT(buffer.begin() != buffer.end());
//
//     StringCache str_cache;
//     constexpr auto strlen_field_size = sizeof(abc::spec::StrLenT);
//     const auto* const begin = buffer.data();       // Inclusive
//     const auto* const end = begin + buffer.size(); // Exclusive
//
//     const auto* ptr = begin;
//     while (ptr < end)
//     {
//         // Get Strlen from buffer:
//         abc::spec::StrLenT strlen = 0;
//         std::memcpy(&strlen, ptr, strlen_field_size);
//         ptr += strlen_field_size;
//
//         // Copy string into continuous buffer and advance:
//         const auto str_buffer_idx = str_cache.str_buffer.size();
//         str_cache.string_data.emplace_back(str_buffer_idx, strlen);
//         const auto last = ptr + strlen;
//         str_cache.str_buffer.insert(str_cache.str_buffer.end(), ptr, last);
//         ptr = last;
//     }
//
//     return str_cache;
// }
//
struct StringCache
{
    std::vector<u8> buffer;
    std::vector<abc::BufferSpan> index_map;
};

int
ABC_Loader::load(const std::vector<u8>& byte_buffer)
{
    const abc::Header header = load_header(byte_buffer);
    //
    // const auto str_lut = header.sections.strings.lut;
    // constexpr auto k_span_size = sizeof(abc::BufferSpan);
    //
    // // Calculate string_tape size:
    // if (str_lut.begin() == str_lut.end())
    //     return 0;
    //
    // DMASSERT(str_lut.begin() + k_span_size <= str_lut.end());
    // abc::BufferSpan first_lut_span;
    // abc::BufferSpan last_lut_span;
    // std::memcpy(&first_lut_span, byte_buffer.data() + str_lut.begin(), k_span_size);
    // std::memcpy(&last_lut_span, byte_buffer.data() + str_lut.end() - k_span_size, k_span_size);
    //
    // StringCache string_cache;
    // DMASSERT(first_lut_span.begin() < last_lut_span.end());
    // string_cache.buffer.insert(
    //     string_cache.buffer.end(),
    //     byte_buffer.data() + first_lut_span.begin(),
    //     byte_buffer.data() + last_lut_span.end()
    // );
    //
    // string_cache.buffer.reserve(last_lut_span.end() - first_lut_span.begin() / k_span_size);
    // for (auto i = str_lut.begin(); i < str_lut.end(); i += k_span_size)
    // {
    //     abc::BufferSpan str_span;
    //     std::memcpy(&str_span, byte_buffer.data(), k_span_size);
    //     string_cache.index_map.push_back(str_span);
    // }

    // const auto str_begin = header.offsets.strings
    // const auto str_cache = load_string_cache(byte_buffer);
}
} // namespace alpha
