#ifndef ALPHA_EXPR_TYPES_HPP
#define ALPHA_EXPR_TYPES_HPP

#include <vector>
#include "alpha_basics.hpp"
#include "alpha_core_types.hpp"
#include "alpha_numeric_types.hpp"
#include "../parser/include/symbols.hpp"

#define IOPCODES_WITH_LABEL \
    X(IF_EQ)                \
    X(IF_NOTEQ)             \
    X(IF_LESS)              \
    X(IF_GREATER)           \
    X(IF_LESSEQ)            \
    X(IF_GREATEREQ)         \
    X(JUMP)

#define IOPCODES_WITHOUT_LABEL \
    X(ASSIGN)                  \
    X(ADD)                     \
    X(SUB)                     \
    X(MUL)                     \
    X(DIV)                     \
    X(MOD)                     \
    X(UMINUS)                  \
    X(AND)                     \
    X(OR)                      \
    X(NOT)                     \
    X(CALL)                    \
    X(PARAM)                   \
    X(RETURN)                  \
    X(GETRETVAL)               \
    X(FUNCSTART)               \
    X(FUNCEND)                 \
    X(TABLECREATE)             \
    X(TABLEGETELEM)            \
    X(TABLESETELEM)

#define ALL_IOPCODES        \
        IOPCODES_WITH_LABEL \
        IOPCODES_WITHOUT_LABEL

namespace Alpha
{
using LabelID = u32;
using AlphaInt = i64;
using AlphaFloat = f64;

enum class IOPCode : u8
{
    #define X(code) code,
    ALL_IOPCODES
    #undef  X
};

enum class OperandSide : u8
{
    UNARY,
    LEFT,
    RIGHT
};

#define EXPR_TYPES      \
    X(ARITHMETIC_EXPR)  \
    X(ASSIGN_EXPR)      \
    X(BOOL_EXPR)        \
    X(CONST_BOOL)       \
    X(CONST_INT)        \
    X(CONST_FLOAT)      \
    X(CONST_STRING)     \
    X(CONST_NIL)        \
    X(LIBRARY_FUNCTION) \
    X(PROGRAM_FUNCTION) \
    X(NEW_TABLE)        \
    X(TABLE_ITEM)       \
    X(VARIABLE)

struct Expr : private Immobile
{
    enum class Type : u8
    {
        #define X(expr_type) expr_type,
        EXPR_TYPES
        #undef  X
    };

    const Type type;
    const SourceLocation loc;

    #ifdef DEBUG_MODE
    const bool has_symbol;
    #endif

    ALWAYS_INLINE Expr(const Type type, const SourceLocation loc, const bool has_symbol = false)
        : type(type),
          loc(loc)
          #ifdef DEBUG_MODE
         ,
          has_symbol(has_symbol)
          #endif
    {}
};

// TODO: Near the end of the project, you would known which fields of Symbols (variable and functions)
// can remain const or not.. if the are mixed up keep as is, and ignore this _TODO_. If not group in a struct.
// an inherit it in each Expr type.

struct ExprWSymbol : public Expr
{
    const Symbol *symbol;

    ALWAYS_INLINE ExprWSymbol(
        const Type type,
        const SourceLocation loc,
        const Symbol *const symbol)
        : Expr(type, loc, true),
          symbol(REQUIRE_PTR(symbol)) {}
};

struct ArithmeticExpr : public ExprWSymbol
{
    ALWAYS_INLINE ArithmeticExpr(const SourceLocation loc, const Symbol *const var_symbol)
        : ExprWSymbol(Type::ARITHMETIC_EXPR, loc, var_symbol)
    {
        // I am not certain, but I think arithmetic expressions are produced only with temp vars.
        DEBUG_SMART_ASSERT(var_symbol->name.starts_with('_')); // TODO remove after you tested.
    }
};

struct AssignExpr : public ExprWSymbol
{
    const Expr *index;

    ALWAYS_INLINE AssignExpr(const SourceLocation loc, const Expr *const rvalue) // NOLINT
        : ExprWSymbol(Type::ASSIGN_EXPR, loc, nullptr), index(nullptr)
    {
        // TODO put require_PTR.
        // TODO: currently this is incomplete. we dont know the type of
        // rvalue. and we nulled symbol and index.. obviously wrong.
        UNIMPLEMENTED(); // Unimplemented because we pass nullptr.
    }
};

struct BoolExpr : public ExprWSymbol
{
    std::vector<LabelID> true_list;
    std::vector<LabelID> false_list;

    ALWAYS_INLINE BoolExpr(const SourceLocation loc, const Symbol *const var_symbol)
        : ExprWSymbol(Type::BOOL_EXPR, loc, var_symbol)
    {
        // I am not certain, but I think arithmetic expressions are produced only with temp vars.
        DEBUG_SMART_ASSERT(var_symbol->name.starts_with('_')); // TODO remove after you tested.
    }
};

struct ConstBoolExpr : public Expr
{
    bool value;

    ConstBoolExpr(const SourceLocation loc, const bool value)
        : Expr(Type::CONST_BOOL, loc),
          value(value) {}
};

struct ConstIntExpr : public Expr
{
    const AlphaInt value;

    ConstIntExpr(const SourceLocation loc, const AlphaInt value)
        : Expr(Type::CONST_INT, loc),
          value(value) {}
};

struct ConstFloatExpr : public Expr
{
    const AlphaFloat value;

    ConstFloatExpr(const SourceLocation loc, const AlphaFloat value)
        : Expr(Type::CONST_FLOAT, loc),
          value(value) {}
};

struct ConstStringExpr : public Expr
{
    const char *value;

    ConstStringExpr(const SourceLocation loc, const char *const value)
        : Expr(Type::CONST_STRING, loc),
          value(Utils::cstrdup(REQUIRE_PTR(value)))
    {
        DEBUG_SMART_ASSERT(!!this->value);
    }

    ~ConstStringExpr()
    {
        DEBUG_SMART_ASSERT(!!value);
        delete [] value;
    }
};

struct ConstNilExpr : public Expr
{
    ConstNilExpr(const SourceLocation loc)
        : Expr(Type::CONST_NIL, loc) {}
};

struct LibFuncExpr : public ExprWSymbol
{
    LibFuncExpr(const SourceLocation loc, const Function *const func_symbol)
        : ExprWSymbol(Type::LIBRARY_FUNCTION, loc, func_symbol) {}
};

struct ProgFuncExpr : public ExprWSymbol
{
    ProgFuncExpr(const SourceLocation loc, const Function *const func_symbol)
        : ExprWSymbol(Type::PROGRAM_FUNCTION, loc, func_symbol) {}
};

struct NewTableExpr : public ExprWSymbol
{
    NewTableExpr(const SourceLocation loc, const Symbol *const var_symbol)
        : ExprWSymbol(Type::NEW_TABLE, loc, var_symbol) {}
};

// TODO: can a table item be a const? or a table item is always a temp (so it always has symbol)
struct TableItemExpr : public ExprWSymbol
{
    const Expr *index;

    TableItemExpr(
        const SourceLocation loc,
        const Symbol *const var_symbol,
        const Expr *const index)
        : ExprWSymbol(Type::TABLE_ITEM, loc, var_symbol),
          index(REQUIRE_PTR(index)) {}
};

struct VariableExpr : public ExprWSymbol
{
    VariableExpr(const SourceLocation loc, const Symbol *const var_symbol)
        : ExprWSymbol(Type::VARIABLE, loc, var_symbol) {}
};

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

[[nodiscard]] inline std::string to_string(const IOPCode iopcode)
{
    switch (iopcode)
    {
    #define X(iopcode)                                                                                 \
    case IOPCode::iopcode: return Utils::str_to_lower(#iopcode);
    ALL_IOPCODES
    #undef  X
    default: UNREACHABLE(FMT::format(
            "BUG: Unknown IOPCode. IOPCode's int value = `{}`", static_cast<int>(iopcode)));
    }
}

inline const char *to_string(const Expr::Type type)
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

inline const char *to_string(const OperandSide pos)
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

inline Expr::Type to_expr_type(const Symbol::Type symbol_type)
{
    switch (symbol_type)
    {
    case Symbol::Type::GLOBAL_VARIABLE:
    case Symbol::Type::FORMAL_ARGUMENT:
    case Symbol::Type::LOCAL_VARIABLE: return Expr::Type::VARIABLE;
    case Symbol::Type::LIBRARY_FUNCTION: return Expr::Type::LIBRARY_FUNCTION;
    case Symbol::Type::PROGRAM_FUNCTION: return Expr::Type::PROGRAM_FUNCTION;
    default: UNREACHABLE("Unknown Symbol::Type");
    }
}
} // namespace Alpha
#undef ALL_IOPCODES
#endif //ALPHA_EXPR_TYPES_HPP
