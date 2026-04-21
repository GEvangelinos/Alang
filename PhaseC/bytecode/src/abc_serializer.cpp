#include "bytecode/abc_serializer.hpp"
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
    constexpr auto k_instruction_size = sizeof(std::decay_t<decltype(program.code)>::value_type);

    const auto total_str_meta_size = sizeof(abc::spec::StrLenT) * program.str_literal_table.size();
    const auto total_instruction_size = k_instruction_size * program.code.size();

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
    const abc::Header::Offsets& offsets)
{
    const abc::Header header{
        .magic = abc::spec::k_magic_value,
        .header_size = sizeof(abc::Header),
        .v_major = 1,
        .v_minor = 2,
        .alignment_pad = 0x00000000,
        .timestamp = get_usec_timestamp(),
        .offsets = offsets,
        .file_size = abc_buffer.size(),
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

static void
serialize_table(std::vector<u8>& abc_buffer, const std::unordered_map<StringSpan, u32>& table)
{
    std::vector<StringSpan> str_literals{table.size()};
    for (const auto [str, idx] : table)
        str_literals[idx] = str;
    for (const StringSpan literal : str_literals)
        serialize_string_content(abc_buffer, literal);
}

static void
serialize_progfuncs(std::vector<u8>& abc_buffer, const vm::Program& program)
{
    for (const vm::Program::UserFunc& func : program.userfuncs)
    {
        serialize_string_content(abc_buffer, func.id);
        store_little_endian(abc_buffer, func.address);
        store_little_endian(abc_buffer, func.local_size);
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
    for (const vm::Instruction& inst : program.code)
    {
        using OpcodeUT = std::underlying_type_t<decltype(inst.opcode)>;
        static_assert(std::is_same_v<OpcodeUT, u8>, "Following push is wrong");
        abc_buffer.push_back(static_cast<OpcodeUT>(inst.opcode));
        if (inst.result)
            serialize_vm_argument(abc_buffer, *inst.result);
        if (inst.arg1)
            serialize_vm_argument(abc_buffer, *inst.arg1);
        if (inst.arg2)
            serialize_vm_argument(abc_buffer, *inst.arg2);
    }
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

    const abc::Header::Offsets offsets{
        .strings = abc_buffer.size(),
        .libfuncs = run([&] { serialize_table(abc_buffer, program.str_literal_table); }),
        .progfuncs = run([&] { serialize_table(abc_buffer, program.libfunc_name_table); }),
        .code = run([&] { serialize_progfuncs(abc_buffer, program); }),
    };
    serialize_instructions(abc_buffer, program);
    serialize_header(abc_buffer, program, offsets);
    return abc_buffer;
}
} // namespace alpha
