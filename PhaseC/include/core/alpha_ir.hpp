#ifndef ALPHA_IR_HPP
#define ALPHA_IR_HPP

#include "parser/alpha_symbols.hpp"
#include "utils/misc.hpp"
#include <string>

// TODO: AFTER PHASE3 change IOPCodes to nicer codes (especially the lengthy ones)

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
        case IOPCode::iopcode: return str_to_lower(#iopcode);
                ALL_IOPCODES
#undef X
        [[unlikely]]
        default:
                throw std::logic_error(
                    ATTACH_CONTEXT(FMT::format("BUG: Unknown IOPCode. IOPCode's int value = `{}`",
                                               static_cast<int>(iopcode))));
        }
}
#undef IOPCODES

enum class OperandPosition : u8
{
        UNARY,
        LEFT,
        RIGHT
};

constexpr const char *to_string(OperandPosition pos)
{
        switch (pos)
        {
        case OperandPosition::LEFT: return "left";
        case OperandPosition::RIGHT:
                return "right";
        [[unlikely]]
        case OperandPosition::UNARY:
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
        Location location;
        union {
                const Expr *index;
                const f64 const_real;
                const i64 const_int;
                const char *const_str;
                const bool const_bool;
        };
        BoolLists *backpatch_info = nullptr; // By default no backpatch_info exists.
};

inline const char *to_string(Expr::Type type)
{
        using AET = Alpha::Expr::Type;
        switch (type)
        {
        case AET::ARITHMETIC_EXPR: return "arithmetic-expression";
        case AET::ASSIGN_EXPR: return "assign-expression";
        case AET::BOOLEAN_EXPR: return "boolean-expression";
        case AET::CONST_BOOL: return "boolean-constant";
        case AET::CONST_INT: return "integer-constant";
        case AET::CONST_NIL: return "nil";
        case AET::CONST_REAL: return "floating-point-constant";
        case AET::CONST_STRING: return "string-literal";
        case AET::LIBRARY_FUNCTION: return "library-function";
        case AET::NEW_TABLE: return "new-table-expression";
        case AET::PROGRAM_FUNCTION: return "program-function";
        case AET::TABLE_ITEM: return "table-item";
        case AET::VARIABLE: return "variable";
        }
}

constexpr Expr::Type to_expr_type(Symbol::Type symbol_type)
{
        switch (symbol_type)
        {
        case Symbol::Type::GLOBAL_VARIABLE:
        case Symbol::Type::FORMAL_ARGUMENT:
        case Symbol::Type::LOCAL_VARIABLE: return Expr::Type::VARIABLE;
        case Symbol::Type::LIBRARY_FUNCTION: return Expr::Type::LIBRARY_FUNCTION;
        case Symbol::Type::PROGRAM_FUNCTION: return Expr::Type::PROGRAM_FUNCTION;
        default: [[unlikely]] SMART_ASSERT(false);
        }
}

// TODO: Which of the following fields can you make const?
// TODO: Also if some fields are initialized only once,
//       but after the construction, create an object,
//       that can be initialized only once (like const),
//       but at arbitrary time.
struct Quad
{
        const IOPCode iopcode;
        const Expr *arg1;
        const Expr *arg2;
        const Expr *result;
        u32 label; // TODO: do we need this? Maybe we can figure this out based on index on vector.
                   // First quad_label is always 1, (0 for backpatching)
        const Location location;
        // We pass location instead of plain line, so we can extract line and column.
        // It will make diagnostic messages in runtime, much more accurate.
};
} // namespace Alpha
#endif // ALPHA_IR_HPP
