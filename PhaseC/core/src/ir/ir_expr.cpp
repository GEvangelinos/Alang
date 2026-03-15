#include <core/ir/ir_expr.hpp>

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
    case ET::ARITHMETIC: return "arithmetic-expression";
    case ET::ASSIGN: return "assign-expression";
    case ET::BOOL: return "bool-expression";
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

bool
Expr::is_rvalue_casted() const noexcept { return rvalue_casted.is_raised(); }

bool
Expr::is_arithmetic_convertible() const noexcept
{
    switch (type)
    {
    case Type::ARITHMETIC:
    case Type::ASSIGN:
    case Type::CONST_INT:
    case Type::CONST_FLOAT:
    case Type::TABLE_ITEM:
    case Type::VARIABLE: return true;
    default: return false;
    }
}

bool
Expr::is_func() const noexcept
{
    return type == Type::LIBRARY_FUNCTION || type == Type::PROGRAM_FUNCTION;
}

bool
Expr::is_bool_or_const_bool() const noexcept
{
    return type == Type::BOOL || type == Type::CONST_BOOL;
}

bool
Expr::is_callable() const noexcept { return is_lvalue() || is_func(); }

bool
Expr::is_const_0() const noexcept
{
    switch (type)
    {
    case Type::CONST_INT: return static_cast<const ConstIntExpr *>(this)->value == 0;
    case Type::CONST_FLOAT: return static_cast<const ConstFloatExpr *>(this)->value == 0.0;
    default: return false;
    }
}

bool
Expr::is_const_1() const noexcept
{
    switch (type)
    {
    case Type::CONST_INT: return static_cast<const ConstIntExpr *>(this)->value == 1;
    case Type::CONST_FLOAT: return static_cast<const ConstFloatExpr *>(this)->value == 1.0;
    default: return false;
    }
}

bool
Expr::is_const_true() const noexcept
{
    return type == Type::BOOL && static_cast<const ConstBoolExpr *>(this)->value == true;
}

bool
Expr::is_const_false() const noexcept
{
    return type == Type::BOOL && static_cast<const ConstBoolExpr *>(this)->value == false;
}

bool
Expr::is_const_arithmetic() const noexcept
{
    return type == Type::CONST_INT || type == Type::CONST_FLOAT;
}

bool
Expr::is_const() const noexcept
{
    switch (type)
    {
    case Type::CONST_BOOL:
    case Type::CONST_INT:
    case Type::CONST_FLOAT:
    case Type::CONST_STRING:
    case Type::CONST_NIL: return true;
    default: return false;
    }
}

bool
Expr::is_lvalue_type() const noexcept
{
    switch (type)
    {
    case Type::ASSIGN:
    case Type::TABLE_ITEM:
    case Type::VARIABLE: return true;
    default: return false;
    }
}

bool
Expr::is_lvalue() const noexcept { return is_lvalue_type() && !is_rvalue_casted(); }

bool
Expr::is_rvalue() const noexcept { return !is_lvalue(); }

bool
Expr::is_static() const noexcept { return is_const() || is_func(); }

bool
Expr::has_symbol() const noexcept { return !is_const(); }

bool
Expr::has_func_symbol() const noexcept { return has_symbol() && is_func(); }

bool
Expr::has_var_symbol() const noexcept { return has_symbol() && !is_func(); }

bool
Expr::has_active_temp() const noexcept
{
    switch (type)
    {
    case Type::ARITHMETIC:
    case Type::ASSIGN:
    case Type::BOOL:
    case Type::NEW_TABLE:
    case Type::TABLE_ITEM:
    case Type::VARIABLE:
        return static_cast<const ExprWVarSymbol *>(this)->var_symbol->has_temp_handle();
    default: return false;
    }
}

Expr::Type
to_expr_type(const Symbol::Type symbol_type)
{
    switch (symbol_type)
    {
    case Symbol::Type::GLOBAL_VARIABLE:
    case Symbol::Type::FORMAL_ARGUMENT:
    case Symbol::Type::LOCAL_VARIABLE: return Expr::Type::VARIABLE;
    case Symbol::Type::LIBRARY_FUNCTION: return Expr::Type::LIBRARY_FUNCTION;
    case Symbol::Type::PROGRAM_FUNCTION: return Expr::Type::PROGRAM_FUNCTION;
    default:
        [[unlikely]] UNREACHABLE(FMT::format(
            "Unknown Symbol::Type. int(symbol_type) = {}", static_cast<int>(symbol_type)));
    }
}
} // namespace alpha
