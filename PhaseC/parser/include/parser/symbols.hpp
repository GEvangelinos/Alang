#ifndef ALPHA_SYMBOLS_HPP
#define ALPHA_SYMBOLS_HPP

#include <list>
#include <string>

#include "_parser_common.hpp"
#include "core/konstants.hpp"
#include "core/source_location.hpp"

namespace alpha
{
struct ConstExpr; // FWD declared
// Classes defined here:
class Symbol;     // IWYU pragma: keep
class VarSymbol;  // IWYU pragma: keep
class FuncSymbol; // IWYU pragma: keep

class Symbol // Lean version (it doesn't contain name, Symbol Table keeps that as its key).
{
public:
    enum class Type : u8
    {
        LIBRARY_FUNCTION,
        PROGRAM_FUNCTION,
        FORMAL_ARGUMENT,
        GLOBAL_VARIABLE,
        LOCAL_VARIABLE,
    };

    const std::string &name;
    const u32 scope;
    const Type type;
    const SourceLocation loc;

    virtual ~Symbol() = default;

    [[nodiscard]] std::string_view type_to_string() const noexcept;
    [[nodiscard]] bool is_variable() const noexcept { return !is_function(); }
    [[nodiscard]] bool is_libfunc() const noexcept;
    [[nodiscard]] bool is_progfunc() const noexcept;
    [[nodiscard]] bool is_function() const noexcept;
    [[nodiscard]] bool is_active() const noexcept { return is_active_; }
    [[nodiscard]] static bool is_modifiable_symbol(const Symbol *symbol);

protected:
    Symbol(const std::string &name, u32 scope, Type type, SourceLocation loc) noexcept;

private:
    bool is_active_ = true;

    DEBUG_ALWAYS_INLINE void activate() noexcept { is_active_ = true; }
    DEBUG_ALWAYS_INLINE void deactivate() noexcept { is_active_ = false; }

    friend class SymbolTable;
};

class VarSymbol final : public Symbol
{
public:
    enum class Space
    {
        PROGRAM_VAR,
        FUNCTION_LOCAL,
        FORMAL_ARGUMENT,
    };

    const Space space;
    const u32 offset;

    VarSymbol(
        const std::string &name,
        u32 scope,
        Type type,
        Space space,
        u32 offset,
        SourceLocation loc) noexcept;
    ~VarSymbol() override = default;

    const ConstExpr *get_const_expr() const noexcept { return const_expr_; }
    bool has_const_value() const noexcept { return const_expr_; }

    [[nodiscard]] static Type scope_to_symbol_type(u32 scope);

private:
    // Used to reference the const_expr in order to extract its const_value for constant_propagation
    // Only modified through friend class SymbolTable!
    mutable const ConstExpr *const_expr_ = nullptr;

    friend class SymbolTable;
};

class FuncSymbol final : public Symbol
{
public:
    const u32 address;
    const std::list<Parameter> parameter_list; // TODO: change to vector (cache friendly...)
    Once<u32> local_variable_count;


    FuncSymbol(
        const std::string &name,
        u32 scope,
        Type type,
        u32 address,
        const std::list<Parameter> &parameter_list,
        SourceLocation location);
    ~FuncSymbol() override = default;

private:
    friend class SymbolTable;
};

inline
Symbol::Symbol(
    const std::string &name,
    const u32 scope,
    const Type type,
    const SourceLocation loc) noexcept
    : name(name), scope(scope), type(type), loc(loc) {}

inline std::string_view
Symbol::type_to_string() const noexcept
{
    switch (type)
    {
    case Type::LIBRARY_FUNCTION: return "LIBRARY_FUNCTION";
    case Type::PROGRAM_FUNCTION: return "PROGRAM_FUNCTION";
    case Type::GLOBAL_VARIABLE: return "GLOBAL_VARIABLE";
    case Type::FORMAL_ARGUMENT: return "FORMAL_ARGUMENT";
    case Type::LOCAL_VARIABLE: return "LOCAL_VARIABLE";
    default: UNREACHABLE("Unexpected Symbol Type.");
    }
}

inline bool
Symbol::is_libfunc() const noexcept { return type == Type::LIBRARY_FUNCTION; }

inline bool
Symbol::is_progfunc() const noexcept { return type == Type::PROGRAM_FUNCTION; }

inline bool
Symbol::is_function() const noexcept { return is_libfunc() || is_progfunc(); }

inline
VarSymbol::VarSymbol(
    const std::string &name,
    const u32 scope,
    const Type type,
    const Space space,
    const u32 offset,
    const SourceLocation loc) noexcept
    : Symbol(name, scope, type, loc),
      space(space),
      offset(offset) {}

inline Symbol::Type VarSymbol::scope_to_symbol_type(const u32 scope)
{
    return scope == k_global_scope ? Type::GLOBAL_VARIABLE : Type::LOCAL_VARIABLE;
}

inline
FuncSymbol::FuncSymbol(
    const std::string &name,
    const u32 scope,
    const Type type,
    const u32 address,
    const std::list<Parameter> &parameter_list,
    const SourceLocation location)
    : Symbol(name, scope, type, location),
      address(address),
      parameter_list(parameter_list)
{
    DEBUG_SMART_ASSERT(
        type == Symbol::Type::LIBRARY_FUNCTION ||
        type == Symbol::Type::PROGRAM_FUNCTION //
    );
}
} // namespace alpha
#endif // ALPHA_SYMBOLS_HPP
