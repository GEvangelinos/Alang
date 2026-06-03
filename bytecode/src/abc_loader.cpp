#include "bytecode/abc_loader.hpp"

#include <concepts>
#include <list>
#include <memory>
#include <vector>
#include <span>

#include "core/fixed_string.hpp"
#include "core/numeric_types.hpp"
#include "core/bytecode/abc_header.hpp"
#include "core/bytecode/vm_program.hpp"
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
    require<"user-funcs expected: 0x{:X} < 0x{:X}">
        (std::less{}, header.sections.progfuncs.offset, buffer.size());
    require<"instructions expected: 0x{:X} < 0x{:X}">
        (std::less{}, header.sections.instructions.offset, buffer.size());

    return header;
}

class StringCache
{
public:
    std::vector<StringSpan> index_map;

    StringCache(const char* const copy_buffer_from, const std::size_t buffer_size)
        : buffer_(std::make_unique_for_overwrite<u8[]>(buffer_size))
    {
        std::memcpy(buffer_.get(), copy_buffer_from, buffer_size);
    }

    StringCache(StringCache&&) noexcept = default;
    StringCache(const StringCache&) = delete;

private:
    std::unique_ptr<u8[]> buffer_;
};

[[nodiscard]] static StringCache
load_string_cache(
    const std::vector<u8>& buffer,
    const abc::Header::Sections::Catalog& string_catalog)
{
    const auto last_span = reinterpret_cast<const abc::BufferSpan*>(
        buffer.data() +
        string_catalog.lut.offset +
        string_catalog.lut.size -
        sizeof(abc::BufferSpan)
    );

    const auto strings_begin_at = string_catalog.lut.offset + string_catalog.lut.size; // Inclusive
    const auto strings_end_at = last_span->offset + last_span->size;                   // Exclusive
    DMASSERT(string_catalog.lut.size % sizeof(abc::BufferSpan) == 0);
    const auto string_count = string_catalog.lut.size / sizeof(abc::BufferSpan);
    const auto buffer_size = strings_end_at - strings_begin_at;

    StringCache result{
        reinterpret_cast<const char*>(buffer.data() + strings_begin_at), buffer_size
    };
    result.index_map.reserve(string_count);


    for (std::size_t i = 0; i < string_catalog.lut.size; i += sizeof(abc::BufferSpan))
    {
        const auto current_buffer_span = *reinterpret_cast<const abc::BufferSpan*>(
            buffer.data() + string_catalog.lut.offset + i);
        result.index_map.emplace_back(
            reinterpret_cast<const char*>(
                buffer.data() + current_buffer_span.offset + sizeof(abc::spec::StrLenT)
            ),
            current_buffer_span.size - sizeof(abc::spec::StrLenT)
        );
    }

    return std::move(result);
}


[[nodiscard]] static std::vector<vm::Program::ProgFunc>
load_progfuncs(
    const std::vector<u8>& buffer,
    const abc::BufferSpan& progfunc_span)
{
    std::vector<vm::Program::ProgFunc> result;
    for (std::size_t i = 0; i < progfunc_span.size; i += sizeof(vm::Program::ProgFunc))
    {
        result.emplace_back();
        vm::Program::ProgFunc progfunc;
        const auto progfunc_index = progfunc_span.offset + i;
        std::memcpy(&result.back(), buffer.data() + progfunc_index, sizeof(progfunc));
    }
    return result;
}

int
ABC_Loader::load(const std::vector<u8>& byte_buffer)
{
    const abc::Header header = load_header(byte_buffer);

    StringCache str_literal_cache = load_string_cache(byte_buffer, header.sections.strings);
    StringCache progfunc_name_cache = load_string_cache(byte_buffer, header.sections.progfunc_names);
    const auto progfuncs = load_progfuncs(byte_buffer, header.sections.progfuncs);

    return 0;
}
} // namespace alpha
