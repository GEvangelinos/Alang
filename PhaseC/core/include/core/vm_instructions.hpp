#ifndef VM_INSTRUCTIONS_HPP
#define VM_INSTRUCTIONS_HPP

#include <array>
#include "numeric_types.hpp"

#define ALPHA_VMOPCODES \
    X(ASSIGN)           \
    X(ADD)              \
    X(SUB)              \
    X(MUL)              \
    X(DIV)              \
    X(MOD)              \
    X(UMINUS)           \
    X(AND)              \
    X(OR)               \
    X(NOT)              \
    X(JEQ)              \
    X(JNE)              \
    X(JLE)              \
    X(JGE)              \
    X(JLT)              \
    X(JGT)              \
    X(CALL)             \
    X(PUSHARG)          \
    X(FUNCENTER)        \
    X(FUNCEXIT)         \
    X(NEWTABLE)         \
    X(TABLEGETELEM)     \
    X(TABLESETELEM)     \
    X(NOP)

namespace alpha::vm
{
enum class Opcode
{
    #define X(vmopcode) vmopcode,
    ALPHA_VMOPCODES
    #undef  X
};

struct Argument
{
    enum class Type
    {
        LABEL,
        GLOBAL,
        FORMAL,
        LOCAL,
        NUMBER,
        STRING,
        BOOLEAN,
        NIL,
        USERFUNC,
        LIBFUNC,
        RETVAL,
    };

    Type type;
    u32 val;
};

struct instruction
{
    vm::Opcode opcode;
    vm::Argument result;
    vm::Argument arg1;
    vm::Argument arg2;
    u32 line;
};

struct Userfunc
{
    u32 address;
    u32 local_size;
    const char *id;
};


constexpr std::array<Opcode, 24> all_vmopcodes_array{
    #define X(vmopcode) Opcode::vmopcode,
    ALPHA_VMOPCODES
    #undef  X
};
} // namespace alpha::vm

#endif // VM_INSTRUCTIONS_HPP
