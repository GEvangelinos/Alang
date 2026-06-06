#include "bytecode/abc_serializer.hpp"

#include <assert.h>
#include <chrono>
#include "core/escape_code_list.hpp"
#include "core/bytecode/abc_spec.hpp"
#include "core/bytecode/abc_header.hpp"
#include "core/libfunc/mappings.hpp"

namespace alpha
{
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
        .padding = {},
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

    abc::spec::StrLenT serialized_len = str.size;
    auto start = str.begin(); // Inclusive
    auto end = str.end();     // Exclusive

    if (*str.begin() == '\"')
    {
        constexpr auto delimiting_quote_count = 2; // 1 left & 1 right
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
    buffer[len_idx + 0] = static_cast<u8>((serialized_len >> (0 * 0)) & 0xFF);
    buffer[len_idx + 1] = static_cast<u8>((serialized_len >> (1 * 8)) & 0xFF);
    buffer[len_idx + 2] = static_cast<u8>((serialized_len >> (2 * 8)) & 0xFF);
    buffer[len_idx + 3] = static_cast<u8>((serialized_len >> (3 * 8)) & 0xFF);
}

[[nodiscard]] static bool
is_zeroed_range(
    const std::vector<u8>& abc_buffer,
    const abc::Header::Sections::Catalog& catalog) noexcept
{
    if (catalog.lut.begin() == catalog.lut.end()) // Empty catalog
        return true;
    const auto range_begin = abc_buffer.begin() + catalog.lut.begin();
    const auto range_end = abc_buffer.begin() + catalog.lut.end();

    const bool cond1 = abc_buffer.size() > catalog.lut.begin();
    const bool cond2 = abc_buffer.size() >= catalog.lut.end();
    const bool cond3 = std::all_of(range_begin, range_end, [](const u8 b) { return b == 0; });
    return !!catalog.lut.offset && cond1 && cond2 && cond3;
}


static void
serialize_catalog(
    std::vector<u8>& abc_buffer,
    const abc::Header::Sections::Catalog& catalog,
    auto serializer,
    const std::size_t item_idx
)
{
    const auto target_span_offset = catalog.lut.begin() + item_idx * sizeof(abc::BufferSpan);

    const auto buff_size_before = abc_buffer.size();
    serializer();
    const auto buff_size_after = abc_buffer.size();
    const auto serialized_string_size = buff_size_after - buff_size_before; // [strlen][str]

    const abc::BufferSpan span{
        .offset = static_cast<decltype(abc::BufferSpan::offset)>(buff_size_before),
        .size = static_cast<decltype(abc::BufferSpan::size)>(serialized_string_size),
    };
    std::memcpy(abc_buffer.data() + target_span_offset, &span, sizeof(span));
}

static void
serialize_table(
    std::vector<u8>& abc_buffer,
    const abc::Header::Sections::Catalog& catalog,
    const std::unordered_map<StringSpan, u32>& table)
{
    DMASSERT(is_zeroed_range(abc_buffer, catalog));
    std::vector<StringSpan> str_literals{table.size()};
    for (const auto [str, idx] : table)
        str_literals[idx] = str;
    for (std::size_t i = 0; i < str_literals.size(); ++i)
    {
        const StringSpan& literal = str_literals[i];
        const auto serializer = [&abc_buffer, literal]()
        {
            serialize_string_content(abc_buffer, literal);
        };
        serialize_catalog(abc_buffer, catalog, serializer, i);
    }
}


// Translate libfunc names to internal IDs


static void
serialize_libfuncs(
    std::vector<u8>& abc_buffer,
    std::unordered_set<StringSpan> libfunc_set)
{
    for (const StringSpan libfunc_name : libfunc_set)
    {
        const std::optional<vm::LibFuncId> libfunc_id = vm::get_libfunc_id(libfunc_name);
        DMASSERT(libfunc_id.has_value());
        const auto libfunc_id_num = static_cast<std::underlying_type_t<vm::LibFuncId>>(*libfunc_id);
        store_little_endian(abc_buffer, libfunc_id_num);
    }
}


static void
serialize_progfuncs(
    std::vector<u8>& abc_buffer,
    const std::vector<vm::ProgFunc>& progfuncs)
{
    for (const vm::ProgFunc& func : progfuncs)
    {
        static_assert(sizeof(func) < 32, "If false capture by reference");
        store_little_endian(abc_buffer, func.name_str_id);
        store_little_endian(abc_buffer, func.address.value);
        store_little_endian(abc_buffer, func.local_size);
    }
}

static void
serialize_vm_argument(std::vector<u8>& buffer, const vm::Argument* const arg)
{
    // Storing Argument Type.
    if (arg == nullptr)
    {
        store_little_endian(buffer, vm::Argument::Type::NONE);
        return;
    }
    store_little_endian(buffer, arg->type);

    // Storing Argument content.
    switch (arg->type)
    {
    #define CASE(ARG_TYPE, ARG_CLASS, ARG_ATTR)\
    case vm::Argument::Type::ARG_TYPE: \
        store_little_endian(buffer, static_cast<const vm::ARG_CLASS *>(arg)->ARG_ATTR); break

    CASE(LABEL, LabelArgument, value.value);
    CASE(GLOBAL, GlobalVariableArgument, offset);
    CASE(FORMAL, FormalVariableArgument, offset);
    CASE(LOCAL, LocalVariableArgument, offset);
    CASE(CONST_BOOL, ConstBoolArgument, value);
    CASE(CONST_INT, ConstIntArgument, value);
    CASE(CONST_FLOAT, ConstFloatArgument, value);
    CASE(CONST_STRING, ConstStringArgument, pool_index);
    CASE(PROGRAMFUNC, ProgramFuncArgument, address.value);
    CASE(LIBFUNC, LibFuncArgument, libfunc_id);

    // Semantic Flags: The type itself carries all necessary information.
    case vm::Argument::Type::CONST_NIL:
    case vm::Argument::Type::RETVAL: break;
    #undef CASE
    }
}

static void
serialize_source_location(std::vector<u8>& abc_buffer, const SourceLocation loc)
{
    store_little_endian(abc_buffer, loc.begin.value);
    store_little_endian(abc_buffer, loc.end.value);
}

static void
serialize_instructions(std::vector<u8>& abc_buffer, const vm::Program& program)
{
    for (const vm::Instruction& inst : program.instructions)
    {
        using OpcodeUT = std::underlying_type_t<decltype(inst.opcode)>;
        static_assert(std::is_same_v<OpcodeUT, u8>, "Following push is wrong");
        abc_buffer.push_back(static_cast<OpcodeUT>(inst.opcode));
        serialize_source_location(abc_buffer, inst.loc);
        serialize_vm_argument(abc_buffer, inst.result.get());
        serialize_vm_argument(abc_buffer, inst.arg1.get());
        serialize_vm_argument(abc_buffer, inst.arg2.get());
    }
}

template <std::ranges::sized_range ContainerType>
[[nodiscard]] static abc::BufferSpan
reserve_lut_span(std::vector<u8>& abc_buffer, const ContainerType& container)
{
    const auto number_of_items = std::ranges::size(container);
    const abc::BufferSpan lut_span{
        // Offset of first BufferSpan (not we add sizeof(lut_span) so we move past the buffer_span indexing lut_span)!!
        .offset = static_cast<decltype(lut_span.offset)>(abc_buffer.size() + sizeof(lut_span)),
        .size = static_cast<decltype(lut_span.size)>(number_of_items * sizeof(abc::BufferSpan)),
    };

    // Serialize lut_span info
    const auto original_size = abc_buffer.size();
    abc_buffer.resize(original_size + sizeof(lut_span));
    std::memcpy(abc_buffer.data() + original_size, &lut_span, sizeof(lut_span));

    // Reserve Space for Section's index region, with zero-initialized bytes.
    abc_buffer.insert(abc_buffer.end(), lut_span.size, 0);
    return lut_span;
}

void add_padding_and_debug_zone(std::vector<u8>& buffer, const u8 alignment = 16)
{
    const auto remainder = buffer.size() % alignment;
    const auto padding = (alignment - remainder) % alignment;
    buffer.insert(buffer.end(), padding, '\0');

    DEBUG(
        buffer.insert(
            buffer.end(),
            {'_', '_', '_', 'D', 'E', 'B', 'U', 'G', '_', 'Z', 'O', 'N', 'E', '_', (u8)padding, '_'}
        );
    )
}

std::vector<u8>
ABC_Serializer::serialize(const vm::Program& program)
{
    std::vector<u8> abc_buffer(sizeof(abc::Header), 0);

    abc::Header::Sections data_sections;
    add_padding_and_debug_zone(abc_buffer);

    // Serialize String literals:
    data_sections.strings = {
        .lut = reserve_lut_span(abc_buffer, program.str_literal_registry.from_key_view())
    };
    serialize_table(
        abc_buffer,
        data_sections.strings,
        program.str_literal_registry.from_key_view()
    );
    add_padding_and_debug_zone(abc_buffer);

    // Serialize progfunc names:
    data_sections.progfunc_names = {
        .lut = reserve_lut_span(abc_buffer, program.progfunc_name_registry.from_key_view())
    };
    serialize_table(
        abc_buffer,
        data_sections.progfunc_names,
        program.progfunc_name_registry.from_key_view()
    );
    add_padding_and_debug_zone(abc_buffer);

    // Serialize progfuncs:
    data_sections.progfuncs.offset = abc_buffer.size();
    serialize_progfuncs(abc_buffer, program.progfuncs);
    data_sections.progfuncs.size = abc_buffer.size() - data_sections.progfuncs.offset;
    add_padding_and_debug_zone(abc_buffer);

    // Serialize Instructions:
    data_sections.instructions.offset = abc_buffer.size();
    serialize_instructions(abc_buffer, program);
    data_sections.instructions.size = abc_buffer.size() - data_sections.instructions.offset;

    add_padding_and_debug_zone(abc_buffer);
    // Backpatch header:
    serialize_header(abc_buffer, program, data_sections);
    return abc_buffer;
}
} // namespace alpha
