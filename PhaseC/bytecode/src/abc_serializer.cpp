#include "bytecode/abc_serializer.hpp"

#include <assert.h>
#include <concepts>
#include <chrono>
#include "core/escape_code_list.hpp"
#include "core/bytecode/abc_spec.hpp"
#include "core/bytecode/abc_header.hpp"

namespace alpha
{
[[nodiscard]] static u64
calculate_serialized_string_table_size(const vm::Program& program) noexcept
{
    const auto total_str_meta_size = sizeof(abc::spec::StrLenT) * program.str_literal_table.size();
    return program.metadata.total_string_size + total_str_meta_size;
}

[[nodiscard]] static u32
calculate_abc_filesize(const vm::Program& program) noexcept
{
    constexpr auto k_instruction_size = sizeof(std::decay_t<decltype(program.instructions
    )>::value_type);

    const auto total_str_meta_size = sizeof(abc::spec::StrLenT) * program.str_literal_table.size();
    const auto total_instruction_size = k_instruction_size * program.instructions.size();

    return sizeof(abc::Header)
           + total_str_meta_size
           + program.metadata.total_string_size
           + total_instruction_size;
}


[[nodiscard]] static u64
get_usec_timestamp() noexcept
{
    const auto time_point = std::chrono::system_clock::now();
    const auto duration = time_point.time_since_epoch();
    const auto seconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    const auto seconds_as_ticks = seconds.count();
    DMASSERT(seconds_as_ticks >= 0);
    return static_cast<u64>(seconds_as_ticks);
}

static void
serialize_header(
    std::vector<u8>& abc_buffer,
    const vm::Program& program,
    const abc::Header::Sections& sections)
{
    const abc::Header header{
        .magic = abc::spec::k_magic_value,
        .header_size = sizeof(abc::Header),
        .v_major = 1,
        .v_minor = 2,
        .alignment_pad = 0x00000000,
        .timestamp = get_usec_timestamp(),
        .file_size = abc_buffer.size(),
        .sections = sections,
    };
    std::memcpy(abc_buffer.data(), &header, sizeof(decltype(header)));
}

template <typename T>
    requires std::is_arithmetic_v<T> || std::is_enum_v<T>
void
store_little_endian(std::vector<u8>& buffer, const T value)
{
    if constexpr (std::is_same_v<T, bool>)
        buffer.push_back(value ? u8{1} : u8{0});
    else
    {
        static_assert(std::endian::native == std::endian::little, "This trick is only for little");
        const auto start = buffer.size();
        buffer.resize(start + sizeof(T));
        std::memcpy(&buffer[start], &value, sizeof(T)); // Usually compiler optimizes to 1 reg MOV
    }
}

static void
serialize_string_content(std::vector<u8>& buffer, const StringSpan str)
{
    DMASSERT(
        !str.empty() && "Even if content is empty there are the delimiters."
        "Strings without the delimiters are made due to object access syntax, which by definition"
        "can not be empty (a name but be there, obj.NAME, where NAME cant be just void, as its a  syntax error)"
    );
    u32 serialized_len = str.size;
    auto start = str.begin(); // Inclusive
    auto end = str.end();     // Exclusive

    if (*str.begin() == '\"')
    {
        constexpr auto delimiting_quote_count = 2; // 1 left & 1 right
        for (const char ch : str)
            std::cout << "CH = " << ch << std::endl;
        DMASSERT(*(str.end() - 1) == '\"', serialized_len >= delimiting_quote_count && "Empty str");
        serialized_len -= delimiting_quote_count;
        ++start;
        --end;
    }

    // We "reserve" the strlen in advance, as any escape code in `str` will decrease `serialized_size` further.
    const auto len_idx = buffer.size();
    buffer.insert(buffer.end(), sizeof(decltype(serialized_len)), 0);

    // Serializing string itself:
    for (const char* ch_addr = start; ch_addr < end; ++ch_addr)
    {
        if (const char ch = *ch_addr; ch != '\\')
        {
            buffer.push_back(ch);
            continue;
        }
        --serialized_len; // We converted a two-letter escape code into a single ascii char.

        DMASSERT(
            ch_addr < end &&
            "As we just saw the first part of the escape code (the backslash) at-least "
            "1 more letter must follow (basically the string can not end here) "
            "(It would result in a lexical failure)."
        );
        switch (*++ch_addr)
        {
        #define RESOLVE_ESCAPE_CODE(ch, escape) case ch: buffer.push_back(escape); break;
        ESCAPE_CODE_LIST(RESOLVE_ESCAPE_CODE)
        #undef RESOLVE_ESCAPE_CODE
        default: DMASSERT(false && "Invalid escape code. How did it survive upto this stage?");
        }
    }

    // Serializing string's length (Explicit Little Endian order, meaning a single read on decoding):
    buffer[len_idx] = static_cast<u8>(serialized_len & 0xFF);
    buffer[len_idx + 1] = static_cast<u8>(serialized_len >> 8 & 0xFF);
    buffer[len_idx + 2] = static_cast<u8>(serialized_len >> 16 & 0xFF);
    buffer[len_idx + 3] = static_cast<u8>(serialized_len >> 24 & 0xFF);
}

[[nodiscard]] static bool
is_zeroed_range(
    const std::vector<u8>& abc_buffer,
    const abc::Header::Sections::Catalog& catalog) noexcept
{
    const auto range_begin = abc_buffer.begin() + catalog.lut.begin();
    const auto range_end = abc_buffer.begin() + catalog.lut.end();

    return !!catalog.lut.offset &&
           abc_buffer.size() > catalog.lut.begin() &&
           abc_buffer.size() > catalog.lut.end() &&
           std::all_of(range_begin, range_end, [](const u8 b) { return b == 0; });
}


static void
serialize_catalog(
    std::vector<u8>& abc_buffer,
    const abc::Header::Sections::Catalog& catalog,
    auto serializer)
{
    auto* const target_span_addr = abc_buffer.data() + catalog.lut.begin();

    const auto buff_size_before = abc_buffer.size();
    serializer();
    const auto buff_size_after = abc_buffer.size();
    const auto serialized_string_size = buff_size_before - buff_size_after;

    using Span = abc::Header::Sections::Span;
    const Span span{
        .offset = static_cast<decltype(Span::offset)>(buff_size_before),
        .size = static_cast<decltype(Span::size)>(serialized_string_size),
    };
    std::memcpy(target_span_addr, &span, sizeof(span));
}

static void
serialize_table(
    std::vector<u8>& abc_buffer,
    const abc::Header::Sections::Catalog& catalog,
    const std::unordered_map<StringSpan, u32>& table)
{
    DMASSERT(is_zeroed_range(abc_buffer,catalog));
    std::vector<StringSpan> str_literals{table.size()};
    for (const auto [str, idx] : table)
        str_literals[idx] = str;
    for (const StringSpan literal : str_literals)
    {
        const auto serializer = [&abc_buffer, literal]()
        {
            serialize_string_content(abc_buffer, literal);
        };
        serialize_catalog(abc_buffer, catalog, serializer);
    }
}

static void
serialize_progfuncs(
    std::vector<u8>& abc_buffer,
    const abc::Header::Sections::Catalog& component,
    const std::vector<vm::Program::ProgFunc>& progfuncs)
{
    DMASSERT(is_zeroed_range(abc_buffer,component));
    for (const vm::Program::ProgFunc& func : progfuncs)
    {
        const auto serializer = [&abc_buffer, func]()
        {
            static_assert(sizeof(func) < 32, "If false capture by reference");
            store_little_endian(abc_buffer, func.address);
            store_little_endian(abc_buffer, func.local_size);
            serialize_string_content(abc_buffer, func.id);
        };
        serialize_catalog(abc_buffer, component, serializer);
    }
}

static void
serialize_vm_argument(std::vector<u8>& buffer, const vm::Argument& arg)
{
    // Storing Argument Type.
    store_little_endian(buffer, arg.type);

    // Storing Argument content.
    switch (arg.type)
    {
    #define CASE(ARG_TYPE, ARG_CLASS, ARG_ATTR)\
    case vm::Argument::Type::ARG_TYPE: \
        store_little_endian(buffer, static_cast<const vm::ARG_CLASS&>(arg).ARG_ATTR); break

    CASE(LABEL, LabelArgument, value.value);
    CASE(GLOBAL, GlobalVariableArgument, offset);
    CASE(FORMAL, FormalVariableArgument, offset);
    CASE(LOCAL, LocalVariableArgument, offset);
    CASE(CONST_BOOL, ConstBoolArgument, value);
    CASE(CONST_INT, ConstIntArgument, value);
    CASE(CONST_FLOAT, ConstFloatArgument, value);
    CASE(CONST_STRING, ConstStringArgument, pool_index);
    CASE(PROGRAMFUNC, ProgramFuncArgument, address);
    CASE(LIBFUNC, LibFuncArgument, pool_index);

    // Semantic Flags: The type itself carries all necessary information.
    case vm::Argument::Type::CONST_NIL:
    case vm::Argument::Type::RETVAL:
        break;
        #undef CASE
    }
}

static void
serialize_instructions(std::vector<u8>& abc_buffer, const vm::Program& program)
{
    for (const vm::Instruction& inst : program.instructions)
    {
        using OpcodeUT = std::underlying_type_t<decltype(inst.opcode)>;
        static_assert(std::is_same_v<OpcodeUT, u8>, "Following push is wrong");
        abc_buffer.push_back(static_cast<OpcodeUT>(inst.opcode));
        if (inst.result) serialize_vm_argument(abc_buffer, *inst.result);
        if (inst.arg1) serialize_vm_argument(abc_buffer, *inst.arg1);
        if (inst.arg2) serialize_vm_argument(abc_buffer, *inst.arg2);
    }
}

template <std::ranges::sized_range ContainerType>
[[nodiscard]] static abc::Header::Sections::Span
reserve_catalog_span(std::vector<u8>& abc_buffer, const ContainerType& container)
{
    using Span = abc::Header::Sections::Span;
    const auto number_of_items = std::ranges::size(container);
    const Span catalog_span{
        .offset = static_cast<decltype(catalog_span.offset)>(abc_buffer.size()),
        .size = static_cast<decltype(catalog_span.size)>(
            number_of_items * sizeof(abc::Header::Sections::Span)),
    };
    // Reserve Space for Section's index region, with zero-initialized bytes.
    abc_buffer.insert(abc_buffer.end(), catalog_span.size, 0);
    return catalog_span;
}

std::vector<u8>
ABC_Serializer::serialize(const vm::Program& program)
{
    std::vector<u8> abc_buffer(sizeof(abc::Header), 0);
    const auto run = [&](auto&& lambda)
    {
        lambda();
        return abc_buffer.size();
    };

    abc::Header::Sections data_sections{
        .strs = {.lut = reserve_catalog_span(abc_buffer, program.str_literal_table)},
        .libfuncs = {.lut = reserve_catalog_span(abc_buffer, program.libfunc_name_table)},
        .progfuncs = {.lut = reserve_catalog_span(abc_buffer, program.progfuncs)},
        .instructions = {},
    };

    serialize_table(abc_buffer, data_sections.strs, program.str_literal_table);
    serialize_table(abc_buffer, data_sections.libfuncs, program.libfunc_name_table);
    serialize_progfuncs(abc_buffer, data_sections.progfuncs, program.progfuncs);

    data_sections.instructions.offset = abc_buffer.size();
    serialize_instructions(abc_buffer, program);
    data_sections.instructions.size = abc_buffer.size() - data_sections.instructions.offset;

    serialize_header(abc_buffer, program, data_sections);
    return abc_buffer;
}
} // namespace alpha
