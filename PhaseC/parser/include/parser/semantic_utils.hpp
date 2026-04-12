#ifndef SEMANTIC_UTILS_HPP
#define SEMANTIC_UTILS_HPP

#include <parser/ir_opcode.gen.hpp>

#include "core/ir/ir_expr.hpp"
#include "support/debug_tools.hpp"
#include "support/smart_assert.h"

namespace alpha::SemUtils
{
[[nodiscard]] constexpr bool is_binary_arithmetic_opcode(const ir::Opcode opc)
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

[[nodiscard]] constexpr bool is_relational_iropcode(const ir::Opcode opc)
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

[[nodiscard]] constexpr bool is_relational_equality_iropcode(const ir::Opcode opc)
{
    return opc == ir::Opcode::IF_EQ || opc == ir::Opcode::IF_NEQ;
}

[[nodiscard]] constexpr bool is_relational_numeric_iropcode(const ir::Opcode opc)
{
    return is_relational_iropcode(opc) && !is_relational_equality_iropcode(opc);
}

[[nodiscard]] constexpr bool is_binary_logical_iropcode(const ir::Opcode opc)
{
    return opc == ir::Opcode::AND || opc == ir::Opcode::OR;
}

[[nodiscard]] constexpr const char* relop_str(const ir::Opcode opc)
{
    DMASSERT(is_relational_iropcode(opc));
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

[[nodiscard]] constexpr const char* arith_op_str(const ir::Opcode opc)
{
    DMASSERT(is_binary_arithmetic_opcode(opc));
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

[[nodiscard]] inline bool as_bool(const Expr* const e) noexcept
{
    DMASSERT(!!e);
    DMASSERT(e->is_static() && "Can only evaluate as bool is given expr is static");
    using ET = Expr::Type;
    switch (e->type)
    {
    case ET::CONST_BOOL: return static_cast<const ConstBoolExpr*>(e)->value;
    case ET::CONST_INT: return static_cast<const ConstIntExpr*>(e)->value != 0;
    case ET::CONST_FLOAT: return static_cast<const ConstFloatExpr*>(e)->value != 0;
    case ET::CONST_STRING: return static_cast<const ConstStringExpr*>(e)->value.size != 0;
    case ET::CONST_NIL: return false;
    case ET::LIBRARY_FUNCTION: return true;
    case ET::PROGRAM_FUNCTION: return true;
    default: UNREACHABLE("Expected bool-convertable expr.");
    }
}

[[nodiscard]] inline AlphaFloat extract_alpha_float(const Expr* const e) noexcept
{
    if (e->type == Expr::Type::CONST_INT)
        return static_cast<const ConstIntExpr*>(e)->value;
    DMASSERT(e->type == Expr::Type::CONST_FLOAT && "Logic error");
    return static_cast<const ConstFloatExpr*>(e)->value;
}
} // namespace alpha::SemanticUtils
#endif // SEMANTIC_UTILS_HPP
