// This file should contain all overloads of to_string for project's type and more
// The generated diagnostic system relies heavily on this file.

#ifndef TO_STRING_HPP
#define TO_STRING_HPP
#include  <string>
#include <parser/ir.hpp>

namespace Alpha
{
[[nodiscard]] inline const std::string &to_string(const std::string &s) { return s; }

[[nodiscard]] inline std::string to_string(const IOPCode iopcode)
{
    switch (iopcode)
    {
    #define X(iopcode) case IOPCode::iopcode: return Utils::str_to_lower(#iopcode);
    ALL_IOPCODES
    #undef  X
    default:
        UNREACHABLE(FMT::format(
            "BUG: Unknown IOPCode. IOPCode's int value = `{}`", static_cast<int>(iopcode)));
    }
}

[[nodiscard]] inline const char *to_string(const Expr::Type type)
{
    using ET = Expr::Type;
    switch (type)
    {
    case ET::ARITHMETIC_EXPR: return "arithmetic-expression";
    case ET::ASSIGN_EXPR: return "assign-expression";
    case ET::BOOL_EXPR: return "bool-expression";
    case ET::CONST_BOOL: return "bool-constant";
    case ET::CONST_INT: return "integer-constant";
    case ET::CONST_NIL: return "nil";
    case ET::CONST_FLOAT: return "floating-point-constant";
    case ET::CONST_STRING: return "string-literal";
    case ET::LIBRARY_FUNCTION: return "library-function";
    case ET::NEW_TABLE: return "new-table-expression";
    case ET::PROGRAM_FUNCTION: return "program-function";
    case ET::TABLE_ITEM: return "table-item";
    case ET::VARIABLE: return "variable";
    default: UNREACHABLE("Unknown Expr::Type");
    }
}

[[nodiscard]] inline const char *to_string(const OperandSide pos)
{
    switch (pos)
    {
    case OperandSide::LEFT: return "left";
    case OperandSide::RIGHT: return "right";
    case OperandSide::UNARY:
        throw std::logic_error(ATTACH_CONTEXT("UNARY has no string representation here"));
    default: throw std::logic_error(ATTACH_CONTEXT("OperandSide unknown."));
    }
}
}
#endif // TO_STRING_HPP
