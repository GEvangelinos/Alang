#ifndef IR_HPP
#define IR_HPP
#include <vector>
#include <parser/konstants.hpp>
#include "core/basics.hpp"
#include "core/numeric_types.hpp"
#include "parser/symbols.hpp"

#include "core/konstants.hpp"
#include "parser/ir_opcode.gen.hpp"

namespace alpha
{
enum class OperandSide : u8
{
    UNARY,
    LEFT,
    RIGHT
};

const char *to_string(OperandSide pos) noexcept;

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

protected:
    ALWAYS_INLINE Expr(const Type type, const SourceLocation loc)
        : type(type), loc(loc) {}
};

const char *to_string(Expr::Type type) noexcept; // We keep outside of Expr, so ADL finds it.

struct ExprWVarSymbol : public Expr
{
public:
    const VarSymbol *const var_symbol;

protected:
    ALWAYS_INLINE ExprWVarSymbol(
        const Type type,
        const SourceLocation loc,
        const VarSymbol *const var_symbol)
        : Expr(type, loc),
          var_symbol(REQUIRE_PTR(var_symbol)) {}
};

struct ExprWFuncSymbol : public Expr
{
public:
    const FuncSymbol *const func_symbol;

protected:
    ALWAYS_INLINE ExprWFuncSymbol(
        const Type type,
        const SourceLocation loc,
        const FuncSymbol *const func_symbol)
        : Expr(type, loc),
          func_symbol(REQUIRE_PTR(func_symbol)) {}
};

struct ConstExpr : public Expr
{
    // Inherit all constructors from Expr, so ConstExpr can be constructed like Expr
    using Expr::Expr;
};

struct ArithmeticExpr final : public ExprWVarSymbol
{
    ALWAYS_INLINE ArithmeticExpr(const SourceLocation loc, const VarSymbol *const var_symbol)
        : ExprWVarSymbol(Type::ARITHMETIC_EXPR, loc, var_symbol) {}
};

struct AssignExpr final : public ExprWVarSymbol
{
    ALWAYS_INLINE AssignExpr(const SourceLocation loc, const VarSymbol *const var_symbol)
        : ExprWVarSymbol(Type::ASSIGN_EXPR, loc, REQUIRE_PTR(var_symbol)) {}
};

struct BoolExpr final : public ExprWVarSymbol
{
    mutable std::vector<LabelID> true_list;
    mutable std::vector<LabelID> false_list;

    ALWAYS_INLINE BoolExpr(const SourceLocation loc, const VarSymbol *const var_symbol)
        : ExprWVarSymbol(Type::BOOL_EXPR, loc, var_symbol) {}
};

struct ConstBoolExpr final : public ConstExpr
{
    bool value;

    ConstBoolExpr(const SourceLocation loc, const bool value)
        : ConstExpr(Type::CONST_BOOL, loc),
          value(value) {}
};

struct ConstIntExpr final : public ConstExpr
{
    const AlphaInt value;

    ConstIntExpr(const SourceLocation loc, const AlphaInt value)
        : ConstExpr(Type::CONST_INT, loc),
          value(value) {}
};

struct ConstFloatExpr final : public ConstExpr
{
    const AlphaFloat value;

    ConstFloatExpr(const SourceLocation loc, const AlphaFloat value)
        : ConstExpr(Type::CONST_FLOAT, loc),
          value(value) {}
};

struct ConstStringExpr final : public ConstExpr
{
    const char *value;

    ConstStringExpr(const SourceLocation loc, const char *const value)
        : ConstExpr(Type::CONST_STRING, loc),
          value(Utils::cstrdup(REQUIRE_PTR(value))) { DEBUG_SMART_ASSERT(!!this->value); }

    ~ConstStringExpr()
    {
        DEBUG_SMART_ASSERT(!!value);
        delete [] value;
    }
};

struct ConstNilExpr final : public ConstExpr
{
    explicit ConstNilExpr(const SourceLocation loc)
        : ConstExpr(Type::CONST_NIL, loc) {}
};

struct LibFuncExpr final : public ExprWFuncSymbol
{
    LibFuncExpr(const SourceLocation loc, const FuncSymbol *const func_symbol)
        : ExprWFuncSymbol(Type::LIBRARY_FUNCTION, loc, func_symbol) {}
};

struct ProgFuncExpr final : public ExprWFuncSymbol
{
    ProgFuncExpr(const SourceLocation loc, const FuncSymbol *const func_symbol)
        : ExprWFuncSymbol(Type::PROGRAM_FUNCTION, loc, func_symbol) {}
};

struct NewTableExpr final : public ExprWVarSymbol
{
    NewTableExpr(const SourceLocation loc, const VarSymbol *const var_symbol)
        : ExprWVarSymbol(Type::NEW_TABLE, loc, var_symbol) {}
};

struct TableItemExpr final : public ExprWVarSymbol
{
    const Expr *index;

    TableItemExpr(
        const SourceLocation loc,
        const VarSymbol *const var_symbol,
        const Expr *const index)
        : ExprWVarSymbol(Type::TABLE_ITEM, loc, REQUIRE_PTR(var_symbol)),
          index(REQUIRE_PTR(index)) {}
};

struct VariableExpr final : public ExprWVarSymbol
{
    VariableExpr(const SourceLocation loc, const VarSymbol *const var)
        : ExprWVarSymbol(Type::VARIABLE, loc, var) { DEBUG_SMART_ASSERT(var->is_variable()); }
};

struct Quad // Physical layout (packed): 8B first, then 4B, then 1B
{
    const SourceLocation location;
    const Expr *result;
    const Expr *arg1;
    const Expr *arg2;
    u32 label; // First quad_label is always 1, (0 for backpatching)
    const ir::Opcode opcode;
};

inline Expr::Type
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

// WARNING: static_* expressions have dummy location which does NOT correspond
// to any real source buffer region. Never return them from synthesis; doing so
// risks invalid ranges in error reporting and location-sensitive computations.
inline static const ConstIntExpr k_static_int_1_expr{k_no_loc, 1};
inline static const ConstBoolExpr k_static_true_expr{k_no_loc, true};
inline static const ConstBoolExpr k_static_false_expr{k_no_loc, false};
} // namespace alpha
#endif // IR_HPP
