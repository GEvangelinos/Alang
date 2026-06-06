#ifndef VM_INSTRUCTION_HPP
#define VM_INSTRUCTION_HPP

#include <memory>

#include "vm_opcodes.hpp"
#include "core/code_address.hpp"
#include "core/machine_types.hpp"
#include "core/numeric_types.hpp"
#include "core/source_location.hpp"
#include "core/libfunc/id.hpp"

namespace alpha::vm
{
struct Argument
{
    enum class Type : u8
    {
        NONE,
        LABEL,
        GLOBAL,
        FORMAL,
        LOCAL,
        CONST_BOOL,
        CONST_INT,
        CONST_FLOAT,
        CONST_STRING,
        CONST_NIL,
        PROGRAMFUNC,
        LIBFUNC,
        RETVAL,
    };

    const Type type;

    explicit Argument(const Type type) : type(type) {}
};

struct VariableArgument : public Argument
{
    const u32 offset;

    VariableArgument(const Argument::Type type, const u32 offset)
        : Argument{type}, offset{offset} {}
};


struct LabelArgument : public Argument
{
    mutable CodeAddress value;

    explicit LabelArgument(const CodeAddress value)
        : Argument{Type::LABEL}, value{value} {}
};

struct GlobalVariableArgument : public VariableArgument
{
    explicit GlobalVariableArgument(const u32 offset)
        : VariableArgument{Type::GLOBAL, offset} {}
};

struct FormalVariableArgument : public VariableArgument
{
    explicit FormalVariableArgument(const u32 offset)
        : VariableArgument{Type::FORMAL, offset} {}
};

struct LocalVariableArgument : public VariableArgument
{
    explicit LocalVariableArgument(const u32 offset)
        : VariableArgument{Type::LOCAL, offset} {}
};

struct ConstBoolArgument : public Argument
{
    const bool value;

    explicit ConstBoolArgument(const bool value)
        : Argument{Type::CONST_BOOL}, value{value} {}
};

struct ConstIntArgument : public Argument
{
    const AlphaInt value;

    explicit ConstIntArgument(const AlphaInt value)
        : Argument{Type::CONST_INT}, value(value) {}
};

struct ConstFloatArgument : public Argument
{
    const AlphaFloat value;

    explicit ConstFloatArgument(const AlphaFloat value)
        : Argument{Type::CONST_FLOAT}, value(value) {}
};

struct ConstStringArgument : public Argument
{
    const u32 pool_index;

    explicit ConstStringArgument(const u32 string_table_index)
        : Argument{Type::CONST_STRING}, pool_index(string_table_index) {}
};

struct ConstNilArgument : public Argument
{
    ConstNilArgument() : Argument{Type::CONST_NIL} {}
};

struct RetvalArgument : public Argument
{
    RetvalArgument() : Argument(Type::RETVAL) {}
};

struct ProgramFuncArgument : public Argument
{
    const CodeAddress address;

    explicit ProgramFuncArgument(const CodeAddress func_address)
        : Argument{Type::PROGRAMFUNC}, address(func_address) {}
};

struct LibFuncArgument : public Argument
{
    const LibFuncId libfunc_id;

    explicit LibFuncArgument(const LibFuncId libfunc_id)
        : Argument{Type::LIBFUNC}, libfunc_id(libfunc_id) {}
};

struct Instruction
{
    const vm::Opcode opcode;
    std::unique_ptr<vm::Argument> result;
    std::unique_ptr<vm::Argument> arg1;
    std::unique_ptr<vm::Argument> arg2;
    const SourceLocation loc;
};
} // namespace alpha::vm

#endif // VM_INSTRUCTION_HPP
