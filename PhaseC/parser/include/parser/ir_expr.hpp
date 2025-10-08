#ifndef IR_HPP
#define IR_HPP
#include <optional>
#include <vector>
#include <parser/konstants.hpp>
#include "core/basics.hpp"
#include "core/numeric_types.hpp"
#include "parser/symbols.hpp"

#include "core/konstants.hpp"
#include "support/misc_tools.hpp"
#include "parser/ir_opcode.gen.hpp"
#include "support/string_tools.hpp"

namespace alpha
{
enum class OperandSide : u8
{
    UNARY,
    LEFT,
    RIGHT
};

const char *to_string(OperandSide pos) noexcept;

#define EXPR_TYPES(X)     \
    X(ARITHMETIC)       \
    X(ASSIGN)           \
    X(BOOL)             \
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

// @Note: Be careful when modifying or reordering fields in this struct.
//        Instances of Expr are generated constantly, so layout and size have a
//        direct impact on performance and memory footprint. Aim to keep the
//        struct as compact as possible.
struct Expr : private Immobile
{
    enum class Type : u8
    {
        #define TYPE_EXTRACTOR(expr_type) expr_type,
        EXPR_TYPES(TYPE_EXTRACTOR)
        #undef  TYPE_EXTRACTOR
    };

    const SourceLocation loc;
    const Type type;

    void rvalue_cast() const;
    [[nodiscard]] bool is_rvalue_casted() const noexcept;
    [[nodiscard]] bool is_arithmetic_convertible() const noexcept;
    [[nodiscard]] bool is_func() const noexcept;
    [[nodiscard]] bool is_bool_or_const_bool() const noexcept;
    [[nodiscard]] bool is_callable() const noexcept;
    [[nodiscard]] bool is_const_0() const noexcept;
    [[nodiscard]] bool is_const_1() const noexcept;
    [[nodiscard]] bool is_const_true() const noexcept;
    [[nodiscard]] bool is_const_false() const noexcept;
    [[nodiscard]] bool is_const_arithmetic() const noexcept;
    [[nodiscard]] bool is_const() const noexcept;
    [[nodiscard]] bool is_lvalue_type() const noexcept;
    [[nodiscard]] bool is_lvalue() const noexcept;
    [[nodiscard]] bool is_rvalue() const noexcept;
    [[nodiscard]] bool is_static() const noexcept;
    [[nodiscard]] bool has_symbol() const noexcept;
    [[nodiscard]] bool has_symbol_func() const noexcept;
    [[nodiscard]] bool has_symbol_var() const noexcept;
    [[nodiscard]] bool has_active_temp() const noexcept;

protected:
    // DO NOT explicitly initialize @param rvalue_casted!
    ALWAYS_INLINE Expr(const Type type, const SourceLocation loc)
        : loc(loc), type(type), rvalue_casted() {}

private:
    mutable OnceFlag rvalue_casted;
};

const char *to_string(Expr::Type type) noexcept; // We keep outside Expr, so ADL finds it.

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
          var_symbol(DEBUG_REQUIRE_PTR(var_symbol)) {}
};

struct ExprWFuncSymbol : public Expr
{
public:
    const FuncSymbol *const func_symbol;

protected:
    ALWAYS_INLINE ExprWFuncSymbol(
        const Type type, const SourceLocation loc, const FuncSymbol *const func_symbol)
        : Expr(type, loc),
          func_symbol(DEBUG_REQUIRE_PTR(func_symbol)) {}
};

struct ConstExpr : public Expr
{
    // Inherit all constructors from Expr, so ConstExpr can be constructed like Expr
    using Expr::Expr;
};

struct ArithmeticExpr final : public ExprWVarSymbol
{
    ALWAYS_INLINE ArithmeticExpr(const SourceLocation loc, const VarSymbol *const var_symbol)
        : ExprWVarSymbol(Type::ARITHMETIC, loc, var_symbol) {}
};

struct AssignExpr final : public ExprWVarSymbol
{
    ALWAYS_INLINE AssignExpr(const SourceLocation loc, const VarSymbol *const var_symbol)
        : ExprWVarSymbol(Type::ASSIGN, loc, DEBUG_REQUIRE_PTR(var_symbol)) {}
};

struct BoolExpr final : public ExprWVarSymbol
{
    mutable std::vector<LabelID> true_list;
    mutable std::vector<LabelID> false_list;

    // We mark as const, as we only change mutable fields.
    void invert() const { std::swap(true_list, false_list); }

    ALWAYS_INLINE BoolExpr(const SourceLocation loc, const VarSymbol *const var_symbol)
        : ExprWVarSymbol(Type::BOOL, loc, var_symbol) {}
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
          value(support::cstrdup(DEBUG_REQUIRE_PTR(value))) { DEBUG_SMART_ASSERT(!!this->value); }

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
        const SourceLocation loc, const VarSymbol *const var_symbol, const Expr *const index)
        : ExprWVarSymbol(Type::TABLE_ITEM, loc, DEBUG_REQUIRE_PTR(var_symbol)),
          index(DEBUG_REQUIRE_PTR(index)) {}
};

struct VariableExpr final : public ExprWVarSymbol
{
    VariableExpr(const SourceLocation loc, const VarSymbol *const var)
        : ExprWVarSymbol(Type::VARIABLE, loc, var) { DEBUG_SMART_ASSERT(var->is_variable()); }
};

struct Quad // Physical layout (packed): 8B first, then 4B, then 1B
{
    SourceLocation loc = {};
    LabelID label = {};
    const Expr *result;
    const Expr *arg1;
    const Expr *arg2;
    const ir::Opcode opcode;
};

inline void
Expr::rvalue_cast() const { rvalue_casted.raise(); }

inline bool
Expr::is_rvalue_casted() const noexcept { return rvalue_casted.is_raised(); }

inline bool
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

inline bool
Expr::is_func() const noexcept
{
    return type == Type::LIBRARY_FUNCTION || type == Type::PROGRAM_FUNCTION;
}

inline bool
Expr::is_bool_or_const_bool() const noexcept
{
    return type == Type::BOOL || type == Type::CONST_BOOL;
}

inline bool
Expr::is_callable() const noexcept { return is_lvalue() || is_func(); }

inline bool
Expr::is_const_0() const noexcept
{
    switch (type)
    {
    case Type::CONST_INT: return static_cast<const ConstIntExpr *>(this)->value == 0;
    case Type::CONST_FLOAT: return static_cast<const ConstFloatExpr *>(this)->value == 0.0;
    default: return false;
    }
}

bool inline
Expr::is_const_1() const noexcept
{
    switch (type)
    {
    case Type::CONST_INT: return static_cast<const ConstIntExpr *>(this)->value == 1;
    case Type::CONST_FLOAT: return static_cast<const ConstFloatExpr *>(this)->value == 1.0;
    default: return false;
    }
}

inline bool
Expr::is_const_true() const noexcept
{
    return type == Type::BOOL && static_cast<const ConstBoolExpr *>(this)->value == true;
}

inline bool
Expr::is_const_false() const noexcept
{
    return type == Type::BOOL && static_cast<const ConstBoolExpr *>(this)->value == false;
}

inline bool
Expr::is_const_arithmetic() const noexcept
{
    return type == Type::CONST_INT || type == Type::CONST_FLOAT;
}

inline bool
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

inline bool
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

inline bool
Expr::is_lvalue() const noexcept { return is_lvalue_type() && !is_rvalue_casted(); }

inline bool
Expr::is_rvalue() const noexcept { return !is_lvalue(); }

inline bool
Expr::is_static() const noexcept { return is_const() || is_func(); }

inline bool
Expr::has_symbol() const noexcept { return !is_const(); }

inline bool
Expr::has_symbol_func() const noexcept { return has_symbol() && is_func(); }

inline bool
Expr::has_symbol_var() const noexcept { return has_symbol() && !is_func(); }

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
