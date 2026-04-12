#ifndef VM_OPCODES_HPP
#define VM_OPCODES_HPP

#include <array>
#include <string>

#include "core/numeric_types.hpp"

#define ALPHA_VMOPCODES(X) \
    X(ASSIGN)              \
    X(ADD)                 \
    X(SUB)                 \
    X(MUL)                 \
    X(DIV)                 \
    X(MOD)                 \
    X(JUMP)                \
    X(JEQ)                 \
    X(JNE)                 \
    X(JLT)                 \
    X(JLE)                 \
    X(JGT)                 \
    X(JGE)                 \
    X(CALLFUNC)            \
    X(PUSHARG)             \
    X(ENTERFUNC)           \
    X(EXITFUNC)            \
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
enum class Opcode : u8
{
    #define AS_ENUM_MEMBER(vmopcode) vmopcode,
    ALPHA_VMOPCODES(AS_ENUM_MEMBER)
    #undef  AS_ENUM_MEMBER
};

constexpr std::array<Opcode, 25> all_vmopcodes_array{
    #define AS_ARRAY_MEMBER(vmopcode) Opcode::vmopcode,
    ALPHA_VMOPCODES(AS_ARRAY_MEMBER)
    #undef  AS_ARRAY_MEMBER
};
[[nodiscard]] std::string to_string(vm::Opcode opc) noexcept;
} // namespace alpha::vm
#endif // VM_OPCODES_HPP
