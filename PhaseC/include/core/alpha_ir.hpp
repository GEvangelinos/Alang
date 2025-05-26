#ifndef ALPHA_IR_HPP
#define ALPHA_IR_HPP

#include <string>
#include "parser/alpha_symbols.hpp"
#include "utils/misc.hpp"

#define IOPCODES_WITH_LABEL                                                                        \
        X(IF_EQ)                                                                                   \
        X(IF_NOTEQ)                                                                                \
        X(IF_LESS)                                                                                 \
        X(IF_GREATER)                                                                              \
        X(IF_LESSEQ)                                                                               \
        X(IF_GREATEREQ)                                                                            \
        X(JUMP)

#define IOPCODES_WITHOUT_LABEL                                                                     \
        X(ASSIGN)                                                                                  \
        X(ADD)                                                                                     \
        X(SUB)                                                                                     \
        X(MUL)                                                                                     \
        X(DIV)                                                                                     \
        X(MOD)                                                                                     \
        X(UMINUS)                                                                                  \
        X(AND)                                                                                     \
        X(OR)                                                                                      \
        X(NOT)                                                                                     \
        X(CALL)                                                                                    \
        X(PARAM)                                                                                   \
        X(RETURN)                                                                                  \
        X(GETRETVAL)                                                                               \
        X(FUNCSTART)                                                                               \
        X(FUNCEND)                                                                                 \
        X(TABLECREATE)                                                                             \
        X(TABLEGETELEM)                                                                            \
        X(TABLESETELEM)

#define ALL_IOPCODES                                                                               \
        IOPCODES_WITH_LABEL                                                                        \
        IOPCODES_WITHOUT_LABEL

namespace Alpha
{
enum class IOPCode : u8
{
#define X(code) code,
    ALL_IOPCODES
#undef X
};

[[nodiscard]] inline std::string to_string(const IOPCode iopcode)
{
    switch (iopcode)
    {
#define X(iopcode)                                                                                 \
        case IOPCode::iopcode: return Utils::str_to_lower(#iopcode);
    ALL_IOPCODES
#undef X
        [[unlikely]] default: UNREACHABLE(FMT::format(
            "BUG: Unknown IOPCode. IOPCode's int value = `{}`",
            static_cast<int>(iopcode)));
    }
}
#undef IOPCODES

enum class OperandSide : u8
{
    UNARY,
    LEFT,
    RIGHT
};

inline const char *to_string(const OperandSide pos)
{
    switch (pos)
    {
    case OperandSide::LEFT: return "left";
    case OperandSide::RIGHT:
        return "right";
        [[unlikely]]
    case OperandSide::UNARY:
        throw std::logic_error(ATTACH_CONTEXT("UNARY has no string representation here"));
        [[unlikely]]
    default:
        throw std::logic_error(ATTACH_CONTEXT("UNARY has no string representation here"));
    }
}

struct BoolLists
{
    std::vector<u32> true_list;
    std::vector<u32> false_list;
};

struct Expr // Tagged Union
{
    enum class Type : u8
    {
        ARITHMETIC_EXPR,
        ASSIGN_EXPR,
        BOOLEAN_EXPR,
        CONST_BOOL,
        CONST_INT,
        CONST_NIL,
        CONST_REAL,
        CONST_STRING,
        LIBRARY_FUNCTION,
        NEW_TABLE,
        PROGRAM_FUNCTION,
        TABLE_ITEM,
        VARIABLE,
    };

    const Type type;
    const Symbol *symbol;
    SourceLocation location;

    union
    {
        const Expr *index;
        const f64 const_real;
        const I64 const_int;
        const char *const_str;
        const bool const_bool;
    };

    BoolLists *backpatch_info = nullptr; // By default, no backpatch_info exists.
};

inline const char *to_string(const Expr::Type type)
{
    using ET = Expr::Type;
    switch (type)
    {
    case ET::ARITHMETIC_EXPR: return "arithmetic-expression";
    case ET::ASSIGN_EXPR: return "assign-expression";
    case ET::BOOLEAN_EXPR: return "boolean-expression";
    case ET::CONST_BOOL: return "boolean-constant";
    case ET::CONST_INT: return "integer-constant";
    case ET::CONST_NIL: return "nil";
    case ET::CONST_REAL: return "floating-point-constant";
    case ET::CONST_STRING: return "string-literal";
    case ET::LIBRARY_FUNCTION: return "library-function";
    case ET::NEW_TABLE: return "new-table-expression";
    case ET::PROGRAM_FUNCTION: return "program-function";
    case ET::TABLE_ITEM: return "table-item";
    case ET::VARIABLE: return "variable";
        [[unlikely]] default: UNREACHABLE("Unknown Expr::Type");
    }
}

inline Expr::Type to_expr_type(const Symbol::Type symbol_type)
{
    switch (symbol_type)
    {
    case Symbol::Type::GLOBAL_VARIABLE:
    case Symbol::Type::FORMAL_ARGUMENT:
    case Symbol::Type::LOCAL_VARIABLE: return Expr::Type::VARIABLE;
    case Symbol::Type::LIBRARY_FUNCTION: return Expr::Type::LIBRARY_FUNCTION;
    case Symbol::Type::PROGRAM_FUNCTION: return Expr::Type::PROGRAM_FUNCTION;
        [[unlikely]] default: UNREACHABLE("Unknown Symbol::Type");
    }
}

struct Quad
{
    const IOPCode iopcode;
    const Expr *arg1;
    const Expr *arg2;
    const Expr *result;
    u32 label;

    // First quad_label is always 1, (0 for backpatching)
    const SourceLocation location;
};
} // namespace Alpha
#endif // ALPHA_IR_HPP
