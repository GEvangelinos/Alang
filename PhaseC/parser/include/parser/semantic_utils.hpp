#ifndef SEMANTIC_UTILS_HPP
#define SEMANTIC_UTILS_HPP

#include "parser/ir_expr.hpp"
#include "parser/ir_opcode.hpp"
#include "utils/smart_assert.h"

namespace alpha::SemUtils
{
[[nodiscard]] inline bool is_arithmetic_convertible_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    using ET = Expr::Type;
    switch (e->type)
    {
    case ET::ARITHMETIC_EXPR:
    case ET::ASSIGN_EXPR:
    case ET::CONST_INT:
    case ET::CONST_FLOAT:
    case ET::TABLE_ITEM:
    case ET::VARIABLE:
        return true;
    default:
        return false;
    }
}

[[nodiscard]] inline bool is_func_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return e->type == Expr::Type::LIBRARY_FUNCTION || e->type == Expr::Type::PROGRAM_FUNCTION;
}

[[nodiscard]] inline bool is_bool_or_const_bool_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return e->type == Expr::Type::BOOL_EXPR || e->type == Expr::Type::CONST_BOOL;
}

[[nodiscard]] inline bool is_const_bool_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return e->type == Expr::Type::CONST_BOOL;
}

[[nodiscard]] bool inline is_const_0(const Expr *const e)
{
    switch (e->type)
    {
    case Expr::Type::CONST_INT: return static_cast<const ConstIntExpr *>(e)->value == 0;
    case Expr::Type::CONST_FLOAT: return static_cast<const ConstFloatExpr *>(e)->value == 0.0;
    default: return false;
    }
}

[[nodiscard]] bool inline is_const_1(const Expr *const e)
{
    switch (e->type)
    {
    case Expr::Type::CONST_INT: return static_cast<const ConstIntExpr *>(e)->value == 1;
    case Expr::Type::CONST_FLOAT: return static_cast<const ConstFloatExpr *>(e)->value == 1.0;
    default: return false;
    }
}

[[nodiscard]] inline bool is_const_true_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return is_const_bool_expr(e) && static_cast<const ConstBoolExpr *>(e)->value == true;
}

[[nodiscard]] inline bool is_const_false_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return is_const_bool_expr(e) && static_cast<const ConstBoolExpr *>(e)->value == false;
}

[[nodiscard]] inline bool is_const_arithmetic_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return e->type == Expr::Type::CONST_INT || e->type == Expr::Type::CONST_FLOAT;
}

[[nodiscard]] inline bool is_const_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    switch (e->type)
    {
    case Expr::Type::CONST_BOOL:
    case Expr::Type::CONST_INT:
    case Expr::Type::CONST_FLOAT:
    case Expr::Type::CONST_STRING:
    case Expr::Type::CONST_NIL: return true;
    default: return false;
    }
}

[[nodiscard]] inline bool is_lvalue_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    switch (e->type)
    {
    case Expr::Type::ASSIGN_EXPR:
    case Expr::Type::TABLE_ITEM:
    case Expr::Type::VARIABLE: return true;
    default: return false;
    }
}

[[nodiscard]] inline bool is_rvalue_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return !is_lvalue_expr(e);
}

[[nodiscard]] inline bool is_static_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return is_const_expr(e) || is_func_expr(e);
}

[[nodiscard]] inline bool is_expr_with_symbol(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return !is_const_expr(e);
}

[[nodiscard]] inline bool is_const_expr_true_or_1(const Expr *const e)
{
    using ET = Expr::Type;
    switch (e->type)
    {
    case ET::CONST_BOOL: return static_cast<const ConstBoolExpr *>(e)->value == true;
    case ET::CONST_FLOAT: return static_cast<const ConstFloatExpr *>(e)->value == 1.0;
    case ET::CONST_INT: return static_cast<const ConstIntExpr *>(e)->value == 1;
    default: return false;
    }
}

[[nodiscard]] constexpr bool is_binary_arithmetic_iropcode(const ir::Opcode opc)
{
    switch (opc)
    {
    case ir::Opcode::ADD:
    case ir::Opcode::SUB:
    case ir::Opcode::MUL:
    case ir::Opcode::DIV:
    case ir::Opcode::MOD:
        return true;
    default:
        return false;
    }
}

[[nodiscard]] inline bool is_relational_iropcode(const ir::Opcode opc)
{
    switch (opc)
    {
    case ir::Opcode::IF_EQ:
    case ir::Opcode::IF_NEQ:
    case ir::Opcode::IF_GT:
    case ir::Opcode::IF_GTE:
    case ir::Opcode::IF_LT:
    case ir::Opcode::IF_LTE:
        return true;
    default:
        return false;
    }
}

[[nodiscard]] inline bool is_relational_equality_iropcode(const ir::Opcode opc)
{
    return opc == ir::Opcode::IF_EQ || opc == ir::Opcode::IF_NEQ;
}

[[nodiscard]] inline bool is_relational_numeric_iropcode(const ir::Opcode opc)
{
    return is_relational_iropcode(opc) && !is_relational_equality_iropcode(opc);
}

[[nodiscard]] constexpr const char *rel_op_to_str(const ir::Opcode opc)
{
    DEBUG_SMART_ASSERT(is_relational_iropcode(opc));
    switch (opc)
    {
    case ir::Opcode::IF_LT: return "<";
    case ir::Opcode::IF_GT: return ">";
    case ir::Opcode::IF_LTE: return "<=";
    case ir::Opcode::IF_GTE: return ">=";
    case ir::Opcode::IF_EQ: return "==";
    case ir::Opcode::IF_NEQ: return "!=";
    default:
        throw std::logic_error(ATTACH_CONTEXT(
            "Expected strictly an ir::Opcode corresponding to a relational operator"));
    }
}

[[nodiscard]] constexpr const char *arith_op_str(const ir::Opcode opc)
{
    DEBUG_SMART_ASSERT(is_binary_arithmetic_iropcode(opc));
    switch (opc)
    {
    case ir::Opcode::ADD: return "+";
    case ir::Opcode::SUB: return "-";
    case ir::Opcode::MUL: return "*";
    case ir::Opcode::DIV: return "/";
    case ir::Opcode::MOD: return "%";
    default:
        throw std::logic_error(ATTACH_CONTEXT(
            "Expected strictly an ir::Opcode corresponding to an arithmetic operator"));
    }
}

[[nodiscard]] inline bool as_bool(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e, is_static_expr(e));
    switch (e->type)
    {
    case Expr::Type::CONST_BOOL: return static_cast<const ConstBoolExpr *>(e)->value;
    case Expr::Type::CONST_INT: return static_cast<const ConstIntExpr *>(e)->value != 0;
    case Expr::Type::CONST_FLOAT: return static_cast<const ConstFloatExpr *>(e)->value != 0;
    case Expr::Type::CONST_STRING:
        return std::strlen(static_cast<const ConstStringExpr *>(e)->value) != 0;
    case Expr::Type::CONST_NIL: return false;
    case Expr::Type::LIBRARY_FUNCTION: return true;
    case Expr::Type::PROGRAM_FUNCTION: return true;
    default: throw std::logic_error(ATTACH_CONTEXT("Expected bool-convertable expr."));
    }
}

[[nodiscard]] inline AlphaFloat extract_alpha_float(const Expr *const e)
{
    if (e->type == Expr::Type::CONST_FLOAT)
        return static_cast<const ConstFloatExpr *>(e)->value;
    if (e->type == Expr::Type::CONST_INT)
        return static_cast<const ConstIntExpr *>(e)->value;
    throw std::logic_error(ATTACH_CONTEXT("Expected CONST_INT or CONST_FLOAT Expr::Type"));
}
} // namespace alpha::SemanticUtils
#endif // SEMANTIC_UTILS_HPP
