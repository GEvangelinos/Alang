#ifndef SEMANTIC_UTILS_HPP
#define SEMANTIC_UTILS_HPP

#include "core/alpha_core_types.hpp"
#include "utils/smart_assert.hpp"

namespace Alpha::SemUtils
{
inline bool is_arithmetic_convertible_expr(const Expr *const e)
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

inline bool is_func_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return e->type == Expr::Type::LIBRARY_FUNCTION || e->type == Expr::Type::PROGRAM_FUNCTION;
}

inline bool is_const_bool_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return e->type == Expr::Type::CONST_BOOL;
}

inline bool is_const_true_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return is_const_bool_expr(e) && static_cast<const ConstBoolExpr *>(e)->value == true;
}

inline bool is_const_false_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return is_const_bool_expr(e) && static_cast<const ConstBoolExpr *>(e)->value == false;
}

inline bool is_const_arithmetic_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return e->type == Expr::Type::CONST_INT || e->type == Expr::Type::CONST_FLOAT;
}

inline bool is_const_expr(const Expr *const e)
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

inline bool is_lvalue_expr(const Expr *const e)
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

inline bool is_rvalue_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return !is_lvalue_expr(e);
}

inline bool is_static_expr(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return is_const_expr(e) || is_func_expr(e);
}

inline bool is_expr_with_symbol(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e);
    return !is_const_expr(e);
}

inline bool is_binary_arithmetic_iopcode(const IOPCode iopc)
{
    switch (iopc)
    {
    case IOPCode::ADD:
    case IOPCode::SUB:
    case IOPCode::MUL:
    case IOPCode::DIV:
    case IOPCode::MOD:
        return true;
    default:
        return false;
    }
}

inline bool is_relational_iopcode(const IOPCode iopc)
{
    switch (iopc)
    {
    case IOPCode::IF_EQ:
    case IOPCode::IF_NOTEQ:
    case IOPCode::IF_GREATER:
    case IOPCode::IF_GREATEREQ:
    case IOPCode::IF_LESS:
    case IOPCode::IF_LESSEQ:
        return true;
    default:
        return false;
    }
}

inline bool is_relational_equality_iopcode(const IOPCode iopc)
{
    return iopc == IOPCode::IF_EQ || iopc == IOPCode::IF_NOTEQ;
}

inline bool is_relational_arithmetic_iopcode(const IOPCode iopc)
{
    return is_relational_iopcode(iopc) && !is_relational_equality_iopcode(iopc);
}

constexpr const char *relop_to_str(const IOPCode iopc)
{
    DEBUG_SMART_ASSERT(is_relational_iopcode(iopc));
    switch (iopc)
    {
    case IOPCode::IF_LESS:
        return "<";
    case IOPCode::IF_GREATER:
        return ">";
    case IOPCode::IF_LESSEQ:
        return "<=";
    case IOPCode::IF_GREATEREQ:
        return ">=";
    case IOPCode::IF_EQ:
        return "==";
    case IOPCode::IF_NOTEQ:
        return "!=";
    default:
        throw std::logic_error(ATTACH_CONTEXT(
            "Expected strictly an IOPCode corresponding to a relational operator"));
    }
}

inline bool iopcode_requires_label(const IOPCode iopc) noexcept
{
    switch (iopc)
    {
#define X(iopcode)                \
    case Alpha::IOPCode::iopcode: \
        return true;
    IOPCODES_WITH_LABEL
#undef X
#define X(iopcode)                \
    case Alpha::IOPCode::iopcode: \
        return false;
    IOPCODES_WITHOUT_LABEL
#undef X
    default:
        UNREACHABLE(
            FMT::format("Unknown IOPCode. IOPCode's int value = {}.", static_cast<int>(iopc)));
    }
}

inline bool as_bool(const Expr *const e)
{
    DEBUG_SMART_ASSERT(!!e, is_static_expr(e));
    switch (e->type)
    {
    case Expr::Type::CONST_BOOL:
        return static_cast<const ConstBoolExpr *>(e)->value;
    case Expr::Type::CONST_INT:
        return static_cast<const ConstIntExpr *>(e)->value != 0;
    case Expr::Type::CONST_FLOAT:
        return static_cast<const ConstFloatExpr *>(e)->value != 0;
    case Expr::Type::CONST_STRING:
        return std::strlen(static_cast<const ConstStringExpr *>(e)->value) != 0;
    case Expr::Type::CONST_NIL:
        return false;
    case Expr::Type::LIBRARY_FUNCTION:
        return true;
    case Expr::Type::PROGRAM_FUNCTION:
        return true;
    default:
        throw std::logic_error(ATTACH_CONTEXT("Expected rvalue expr."));
    }
}
} // namespace Alpha::SemanticUtils
#endif // SEMANTIC_UTILS_HPP
