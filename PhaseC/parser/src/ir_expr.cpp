#include <parser/ir_expr.hpp>

namespace alpha
{
const char *
to_string(const OperandSide pos) noexcept
{
    switch (pos)
    {
    case OperandSide::LEFT: return "left";
    case OperandSide::RIGHT: return "right";
    case OperandSide::UNARY: SMART_ASSERT(false && "UNARY has no string representation here");
    }
    UNREACHABLE(FMT::format( "Unknown OperandSide. int(pos) = {}", static_cast<int>(pos)));
}

const char *
to_string(const Expr::Type type) noexcept
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
    default:
        [[unlikely]] UNREACHABLE(FMT::format(
            "Unknown Expr::Type. int(type) = {}", static_cast<int>(type)));
    }
}
} // namespace alpha
