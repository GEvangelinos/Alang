#ifndef ALPHA_SYMBOLS_HPP
#define ALPHA_SYMBOLS_HPP

#include <list>
#include  <optional>
#include <string>

#include "_parser_common.hpp"
#include "core/konstants.hpp"
#include "core/source_location.hpp"
#include "parser/internal_typedefs.hpp"
#include "support/misc_tools.hpp"

namespace alpha
{
struct ConstExpr; // FWD declared
// Classes defined here:
class Symbol;     // IWYU pragma: keep
class VarSymbol;  // IWYU pragma: keep
class FuncSymbol; // IWYU pragma: keep

class Symbol // Lean version (it doesn't contain name, Symbol Table keeps that as its key).
{
    friend class SymbolTable;

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
    [[nodiscard]] bool is_temp_variable() const noexcept;
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

    void activate() noexcept { is_active_ = true; }
    void deactivate() noexcept { is_active_ = false; }
};

class VarSymbol final : public Symbol
{
    friend class SymbolTable;

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

    [[nodiscard]] const ConstExpr *get_const_expr() const noexcept { return const_expr_; }
    [[nodiscard]] bool has_const_value() const noexcept { return const_expr_; }
    [[nodiscard]] bool has_temp_handle() const noexcept { return temp_binding_.is_active(); }
    [[nodiscard]] TempHandleID temp_handle() const noexcept;

    [[nodiscard]] static Type scope_to_symbol_type(u32 scope);

private:
    class TempBinding final
    {
    public:
        enum class Status :u8 { ACQUIRED, RELEASED };

        [[nodiscard]] bool is_active() const noexcept { return status_ == Status::ACQUIRED; }
        [[nodiscard]] TempHandleID id() const noexcept;
        void bind(TempHandleID id) noexcept;
        TempHandleID release() noexcept;

    private:
        TempHandleID id_ = std::numeric_limits<TempHandleID>::max();
        Status status_ = Status::RELEASED;
    };

    // Used to reference the const_expr in order to extract its const_value for constant_propagation
    // Only modified through friend class SymbolTable!
    mutable const ConstExpr *const_expr_ = nullptr;
    mutable TempBinding temp_binding_;
};

class FuncSymbol final : public Symbol
{
    friend class SymbolTable;

public:
    const u32 address;
    const std::vector<Parameter> parameter_list;

    // Declared mutable, as we backpatch it after the function’s complete definition.
    mutable Once<u32> stackframe_slot_count;

    FuncSymbol(
        const std::string &name,
        u32 scope,
        Type type,
        u32 address,
        const std::vector<Parameter> &parameter_list,
        SourceLocation location);
    ~FuncSymbol() override = default;
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
Symbol::is_temp_variable() const noexcept
{
    DEBUG(if (name.starts_with(k_temp_variable_prefix)) SMART_ASSERT(is_variable());)
    return name.starts_with(k_temp_variable_prefix);
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

inline Symbol::Type
VarSymbol::scope_to_symbol_type(const u32 scope)
{
    return scope == k_global_scope ? Type::GLOBAL_VARIABLE : Type::LOCAL_VARIABLE;
}

inline TempHandleID
VarSymbol::temp_handle() const noexcept
{
    DEBUG_SMART_ASSERT(has_temp_handle() && "Variable symbol has no temp_handle to return");
    return temp_binding_.id();
}

inline void
VarSymbol::TempBinding::bind(const TempHandleID id) noexcept
{
    id_ = id;
    status_ = Status::ACQUIRED;
}

inline TempHandleID
VarSymbol::TempBinding::release() noexcept
{
    status_ = Status::RELEASED;
    return id_;
}

inline TempHandleID
VarSymbol::TempBinding::id() const noexcept
{
    DEBUG_SMART_ASSERT(is_active());
    return id_;
}

inline
FuncSymbol::FuncSymbol(
    const std::string &name,
    const u32 scope,
    const Type type,
    const u32 address,
    const std::vector<Parameter> &parameter_list,
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
