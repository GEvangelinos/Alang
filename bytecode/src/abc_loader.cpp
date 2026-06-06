#include "bytecode/abc_loader.hpp"

#include <concepts>
#include <list>
#include <memory>
#include <vector>
#include <span>

#include "core/fixed_string.hpp"
#include "core/numeric_types.hpp"
#include "core/string_cache,hpp.h"
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

template <typename T>
[[nodiscard]] T
memread(const alpha::u8* src)
{
    T value;
    std::memcpy(&value, src, sizeof(value));
    return value;
}

template <typename T>
[[nodiscard]] T
memread_and_advance(const alpha::u8*& src)
{
    std::remove_const_t<T> value;
    constexpr auto value_size = sizeof(value);
    std::memcpy(&value, src, value_size);
    src += value_size;
    return value;
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

[[nodiscard]] static vm::StringCache
load_string_cache(
    const std::vector<u8>& buffer,
    const abc::Header::Sections::Catalog& string_catalog)
{
    const u8* const buffer_span_target =
        buffer.data() +
        string_catalog.lut.offset +
        string_catalog.lut.size -
        sizeof(abc::BufferSpan);
    const auto last_span = memread<abc::BufferSpan>(buffer_span_target);

    const auto strings_begin_at = string_catalog.lut.offset + string_catalog.lut.size; // Inclusive
    const auto strings_end_at = last_span.offset + last_span.size;                     // Exclusive
    DMASSERT(string_catalog.lut.size % sizeof(abc::BufferSpan) == 0);
    const auto string_count = string_catalog.lut.size / sizeof(abc::BufferSpan);
    const auto buffer_size = strings_end_at - strings_begin_at;

    vm::StringCache result{
        reinterpret_cast<const char*>(buffer.data() + strings_begin_at), buffer_size
    };
    result.index_map.reserve(string_count);


    for (std::size_t i = 0; i < string_catalog.lut.size; i += sizeof(abc::BufferSpan))
    {
        const u8* const current_buffer_span_addr = buffer.data() + string_catalog.lut.offset + i;
        const auto current_buffer_span = memread<abc::BufferSpan>(current_buffer_span_addr);
        result.index_map.emplace_back(
            reinterpret_cast<const char*>(
                buffer.data() + current_buffer_span.offset + sizeof(abc::spec::StrLenT)
            ),
            current_buffer_span.size - sizeof(abc::spec::StrLenT)
        );
    }

    return std::move(result);
}


[[nodiscard]] static std::vector<vm::ProgFuncMetadata>
load_progfuncs(
    const std::vector<u8>& buffer,
    const abc::BufferSpan& progfunc_span)
{
    std::vector<vm::ProgFuncMetadata> result;
    for (std::size_t i = 0; i < progfunc_span.size; i += sizeof(vm::ProgFuncMetadata))
    {
        result.emplace_back();
        const auto progfunc_index = progfunc_span.offset + i;
        std::memcpy(&result.back(), buffer.data() + progfunc_index, sizeof(vm::ProgFuncMetadata));
    }
    return result;
}

[[nodiscard]] std::unique_ptr<vm::Argument>
load_argument(const u8*& addr)
{
    const auto arg_type = memread_and_advance<vm::Argument::Type>(addr);
    switch (arg_type)
    {
    case vm::Argument::Type::NONE: return nullptr;
    case vm::Argument::Type::LABEL:
        {
            const CodeAddress label{memread_and_advance<CodeAddress::UnderlyingType>(addr)};
            return std::make_unique<vm::LabelArgument>(label);
        }
    case vm::Argument::Type::GLOBAL:
        return std::make_unique<vm::GlobalVariableArgument>(memread_and_advance<u32>(addr));
    case vm::Argument::Type::FORMAL:
        return std::make_unique<vm::FormalVariableArgument>(memread_and_advance<u32>(addr));
    case vm::Argument::Type::LOCAL:
        return std::make_unique<vm::LocalVariableArgument>(memread_and_advance<u32>(addr));
    case vm::Argument::Type::CONST_BOOL:
        return std::make_unique<vm::ConstBoolArgument>(memread_and_advance<bool>(addr));
    case vm::Argument::Type::CONST_INT:
        return std::make_unique<vm::ConstIntArgument>(memread_and_advance<AlphaInt>(addr));
    case vm::Argument::Type::CONST_FLOAT:
        return std::make_unique<vm::ConstFloatArgument>(memread_and_advance<AlphaFloat>(addr));
    case vm::Argument::Type::CONST_STRING:
        return std::make_unique<vm::ConstStringArgument>(memread_and_advance<u32>(addr));
    case vm::Argument::Type::PROGRAMFUNC:
        {
            const auto func_idx =
                memread_and_advance<decltype(vm::ProgramFuncArgument::func_idx)>(addr);
            return std::make_unique<vm::ProgramFuncArgument>(func_idx);
        }
    case vm::Argument::Type::LIBFUNC:
        {
            using LibFuncIdUT = std::underlying_type_t<vm::LibFuncId>;
            const vm::LibFuncId lib_id{memread_and_advance<LibFuncIdUT>(addr)};
            return std::make_unique<vm::LibFuncArgument>(lib_id);
        }
    case vm::Argument::Type::RETVAL: return std::make_unique<vm::RetvalArgument>();
    case vm::Argument::Type::CONST_NIL: return std::make_unique<vm::ConstNilArgument>();
    }
}

[[nodiscard]] SourceLocation
load_source_location(const u8*& addr)
{
    const auto loc_begin = memread_and_advance<SrcBuffIdx::UnderlyingType>(addr);
    const auto loc_end = memread_and_advance<SrcBuffIdx::UnderlyingType>(addr);
    return SourceLocation{SrcBuffIdx{loc_begin}, SrcBuffIdx{loc_end}};
}

[[nodiscard]] static std::vector<vm::Instruction>
load_instructions(
    const std::vector<u8>& buffer,
    const abc::BufferSpan& instruction_span)
{
    std::vector<vm::Instruction> loaded_instructions;
    const u8* addr = buffer.data() + instruction_span.offset;
    const u8* const instructions_end_at = addr + instruction_span.size;
    while (addr < instructions_end_at)
    {
        // Find instruction type (Opcode):
        const vm::Opcode opcode = memread_and_advance<vm::Opcode>(addr);
        const SourceLocation loc = load_source_location(addr);
        auto result = load_argument(addr); // RESULT
        auto arg1 = load_argument(addr);   // ARG1
        auto arg2 = load_argument(addr);   // ARG2
        loaded_instructions.emplace_back(
            opcode,
            std::move(result),
            std::move(arg1),
            std::move(arg2),
            loc
        );
    }
    return loaded_instructions;
}

[[nodiscard]] std::string to_string(const CodeAddress label)
{
    return label.is_none() ? "NONE" : FMT::to_string(label.value);
}

std::string argument_formatter(
    const alpha::vm::Argument* const a,
    const char* const missing_marker)
{
    using namespace alpha;
    if (!a)
        return missing_marker;
    using AT = vm::Argument::Type;

    switch (a->type)
    {
    case AT::CONST_BOOL:
        return static_cast<const vm::ConstBoolArgument*>(a)->value ? "true" : "false";
    case AT::CONST_INT:
        return FMT::format("(int){}", static_cast<const vm::ConstIntArgument*>(a)->value);
    case AT::CONST_FLOAT:
        return FMT::format("(float){}", static_cast<const vm::ConstFloatArgument*>(a)->value);
    case AT::CONST_STRING:
        return FMT::format("(str){}", static_cast<const vm::ConstStringArgument*>(a)->pool_index);
    case AT::CONST_NIL:
        return "nil";
    case vm::Argument::Type::LABEL:
        return to_string(static_cast<const vm::LabelArgument*>(a)->value);
    case vm::Argument::Type::GLOBAL:
        return FMT::format("(global){}", static_cast<const vm::GlobalVariableArgument*>(a)->offset);
    case vm::Argument::Type::FORMAL:
        return FMT::format("(formal){}", static_cast<const vm::FormalVariableArgument*>(a)->offset);
    case vm::Argument::Type::LOCAL:
        return FMT::format("(local){}", static_cast<const vm::LocalVariableArgument*>(a)->offset);
    case vm::Argument::Type::PROGRAMFUNC:
        return FMT::format(
            "(progfunc){}", static_cast<const vm::ProgramFuncArgument*>(a)->func_idx
        );
    case vm::Argument::Type::LIBFUNC:
        return FMT::format(
            "(libfunc){}",
            static_cast<std::underlying_type_t<vm::LibFuncId>>(
                static_cast<const vm::LibFuncArgument*>(a)->libfunc_id
            )
        );
    case vm::Argument::Type::RETVAL:
        return "(retval)";
    default:
        UNREACHABLE(FMT::format("Unhandled vm::Argument::Type: int({}) = {}",
            TO_STRING(expr->type), static_cast<int>(a->type)
        ));
    }
}

vm::Executable
ABC_Loader::load(const std::vector<u8>& byte_buffer)
{
    const abc::Header header = load_header(byte_buffer);

    vm::StringCache str_literal_cache = load_string_cache(
        byte_buffer, header.sections.strings);
    vm::StringCache progfunc_name_cache =
        load_string_cache(byte_buffer, header.sections.progfunc_names);
    std::vector<vm::ProgFuncMetadata> progfuncs =
        load_progfuncs(byte_buffer, header.sections.progfuncs);
    std::vector<vm::Instruction> instructions =
        load_instructions(byte_buffer, header.sections.instructions);

    for (const auto& inst : instructions)
    {
        std::cout << argument_formatter(inst.result.get(), "-");
        std::cout << "\t";
        std::cout << argument_formatter(inst.arg1.get(), "-");
        std::cout << "\t";
        std::cout << argument_formatter(inst.arg2.get(), "-");
        std::cout << std::endl;
    }

    return vm::Executable{
        .str_literal_cache = std::move(str_literal_cache),
        .progfunc_name_cache = std::move(progfunc_name_cache),
        .progfuncs = std::move(progfuncs),
        .instructions = std::move(instructions),
    };
}
} // namespace alpha