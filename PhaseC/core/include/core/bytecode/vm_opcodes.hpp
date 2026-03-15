#ifndef VM_OPCODES_HPP
#define VM_OPCODES_HPP

#include <array>

#define ALPHA_VMOPCODES(X) \
    X(ASSIGN)              \
    X(ADD)                 \
    X(SUB)                 \
    X(MUL)                 \
    X(DIV)                 \
    X(MOD)                 \
    X(JEQ)                 \
    X(JNE)                 \
    X(JLE)                 \
    X(JGE)                 \
    X(JLT)                 \
    X(JGT)                 \
    X(CALL)                \
    X(PUSHARG)             \
    X(FUNCENTER)           \
    X(FUNCEXIT)            \
    X(NEWTABLE)            \
    X(TABLEGETELEM)        \
    X(TABLESETELEM)        \
    X(NOP)                 \
                           \
    X(UMINUS)              \
    X(AND)                 \
    X(OR)                  \
    X(NOT)

namespace alpha::vm
{
enum class Opcode
{
    #define AS_ENUM_MEMBER(vmopcode) vmopcode,
    ALPHA_VMOPCODES(AS_ENUM_MEMBER)
    #undef  AS_ENUM_MEMBER
};

constexpr std::array<Opcode, 24> all_vmopcodes_array{
    #define AS_ARRAY_MEMBER(vmopcode) Opcode::vmopcode,
    ALPHA_VMOPCODES(AS_ARRAY_MEMBER)
    #undef  AS_ARRAY_MEMBER
};
} // namespace alpha::vm
#endif // VM_OPCODES_HPP
