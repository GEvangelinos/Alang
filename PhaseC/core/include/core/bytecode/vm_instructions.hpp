#ifndef VM_INSTRUCTIONS_HPP
#define VM_INSTRUCTIONS_HPP

#include "vm_opcodes.hpp"
#include "core/numeric_types.hpp"
#include "core/source_location.hpp"

namespace alpha::vm
{
struct Argument
{
    enum class Type : u8
    {
        LABEL,
        GLOBAL,
        FORMAL,
        LOCAL,
        CONST_BOOL,
        CONST_INT,
        CONST_FLOAT,
        CONST_STRING,
        CONST_NIL,
        USERFUNC,
        LIBFUNC,
        RETVAL,
    };

    const Type type;

    explicit Argument(const Type type) : type(type) {}
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
        : Argument{Type::CONST_INT}, value(value) {}
};

struct ConstStringArgument : public Argument
{
    const u32 table_index;

    explicit ConstStringArgument(const u32 string_table_index)
        : Argument{Type::CONST_STRING}, table_index(string_table_index) {}
};

struct ConstNilArgument : public Argument
{
    ConstNilArgument() : Argument{Type::CONST_NIL} {}
};

struct VariableArgument : public Argument
{
protected:
    const u32 offset;

    VariableArgument(const Argument::Type type, const u32 offset)
        : Argument{type}, offset{offset} {}
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

struct ProgramFuncArgument : public Argument
{
    const u32 address;

    explicit ProgramFuncArgument(const u32 func_address)
        : Argument{Type::LOCAL}, address(func_address) {}
};

struct LibFuncArgument : public Argument
{
    const u32 pool_index;

    explicit LibFuncArgument(const u32 pool_index)
        : Argument{Type::LOCAL}, pool_index(pool_index) {}
};

struct Instruction
{
    const vm::Opcode opcode;
    const vm::Argument* const result;
    const vm::Argument* const arg1;
    const vm::Argument* const arg2;
    SourceLocation loc;
};
} // namespace alpha::vm

#endif // VM_INSTRUCTIONS_HPP
