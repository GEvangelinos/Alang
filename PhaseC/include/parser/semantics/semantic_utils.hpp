//
// Created by stygian on 5/25/25.
//

#ifndef SEMANTIC_UTILS_HPP
#define SEMANTIC_UTILS_HPP

#include  "core/alpha_ir.hpp"

namespace Alpha::SemUtils
{
inline bool is_const_number_expr(const Expr *const expr)
{
        return expr->type == Expr::Type::CONST_INT ||
               expr->type == Expr::Type::CONST_REAL;
}

inline bool is_numeric_convertible_expr(const Expr *const expr)
{
        using ET = Expr::Type;
        switch(expr->type)
        {
        case ET::ARITHMETIC_EXPR:
        case ET::ASSIGN_EXPR:
        case ET::CONST_INT:
        case ET::CONST_REAL:
        case ET::TABLE_ITEM:
        case ET::VARIABLE: return true;
        default: return false;
        }
}

inline bool is_equality_iopcode(const IOPCode iopc)
{
        return iopc == IOPCode::IF_EQ || iopc == IOPCode::IF_NOTEQ;
}

inline bool is_rvalue_expr(const Expr::Type type)
{
        using ET = Expr::Type;
        switch(type)
        {
        case ET::CONST_BOOL:
        case ET::CONST_INT:
        case ET::CONST_NIL:
        case ET::CONST_REAL:
        case ET::CONST_STRING:
        case ET::LIBRARY_FUNCTION:
        case ET::PROGRAM_FUNCTION: return true;
        default: return false;
        }
}

inline bool is_relational_iopcode(const IOPCode iopc)
{
        switch(iopc)
        {
        case IOPCode::IF_EQ:
        case IOPCode::IF_NOTEQ:
        case IOPCode::IF_GREATER:
        case IOPCode::IF_GREATEREQ:
        case IOPCode::IF_LESS:
        case IOPCode::IF_LESSEQ: return true;
        default: return false;
        }
}

constexpr const char *relational_iopcode_to_string_symbol(const IOPCode iopc)
{
        DEBUG_SMART_ASSERT(is_relational_iopcode(iopc));
        switch(iopc)
        {
        case IOPCode::IF_LESS: return "<";
        case IOPCode::IF_GREATER: return ">";
        case IOPCode::IF_LESSEQ: return "<=";
        case IOPCode::IF_GREATEREQ: return ">=";
        case IOPCode::IF_EQ: return "==";
        case IOPCode::IF_NOTEQ: return "!=";
        default:
                throw std::logic_error(ATTACH_CONTEXT(
                        "Expected strictly an IOPCode corresponding to a relational operator"));
        }
}
} // namespace Alpha::SemanticUtils
#endif //SEMANTIC_UTILS_HPP
