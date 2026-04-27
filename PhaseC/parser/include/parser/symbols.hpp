#ifndef ALPHA_SYMBOLS_HPP
#define ALPHA_SYMBOLS_HPP

#include <list>
#include  <optional>
#include <string>

#include "_parser_common.hpp"
#include "core/konstants.hpp"
#include "core/source_location.hpp"
#include "core/string_span.hpp"
#include "core/temp_handle_id.hpp"
#include "parser/internal_typedefs.hpp"
#include "support/misc_tools.hpp"

namespace alpha
{
struct ConstExpr; // FWD declared
// Classes defined here:
class Symbol;         // IWYU pragma: keep
class VarSymbol;      // IWYU pragma: keep
class ProgFuncSymbol; // IWYU pragma: keep

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

    const StringSpan name;
    const u32 scope;
    const Type type;
    const SourceLocation loc;

    virtual ~Symbol() = default;

    [[nodiscard]] std::string_view type_to_string() const noexcept;
    [[nodiscard]] bool has_tempvar_name() const noexcept;
    [[nodiscard]] bool is_variable() const noexcept { return !is_function(); }
    [[nodiscard]] bool is_function() const noexcept;
    [[nodiscard]] bool is_active() const noexcept { return is_active_; }

protected:
    Symbol(StringSpan name, u32 scope, Type type, SourceLocation loc) noexcept;

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
        StringSpan name,
        u32 scope,
        Type type,
        Space space,
        u32 offset,
        SourceLocation loc,
        bool is_temp) noexcept;
    ~VarSymbol() override = default;

    [[nodiscard]] const ConstExpr* get_const_expr() const noexcept { return const_expr_; }
    [[nodiscard]] bool has_const_value() const noexcept { return const_expr_; }
    [[nodiscard]] bool has_temp_handle() const noexcept { return temp_binding_.is_active(); }
    [[nodiscard]] bool is_initialized() const noexcept { return is_initialized_; }
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
    mutable const ConstExpr* const_expr_ = nullptr;
    mutable TempBinding temp_binding_;
    mutable OnceFlag is_initialized_;

public:
    const bool is_temp;
};

class LibFuncSymbol final : public Symbol
{
    friend class SymbolTable;

public:
    // Constructor keeps a dummy 'scope' parameter for compatibility
    // with the generic SymbolTable::insert_symbol() template.
    explicit LibFuncSymbol(StringSpan name, [[maybe_unused]] u32);
    ~LibFuncSymbol() override = default;
};

class ProgFuncSymbol final : public Symbol
{
    friend class SymbolTable;

public:
    const u32 address;
    const std::vector<Parameter> parameter_list;

    // Declared mutable, as we backpatch it after the function’s complete definition.
    mutable Once<u32> stackframe_slot_count;
    // TODO: Is this required? (if you remove it... does it break anything?)

    ProgFuncSymbol(
        StringSpan name,
        u32 scope,
        u32 address,
        const std::vector<Parameter>& parameter_list,
        SourceLocation location);
    ~ProgFuncSymbol() override = default;
};

inline
Symbol::Symbol(
    const StringSpan name,
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
Symbol::has_tempvar_name() const noexcept
{
    const bool has_temp_prefix = name.to_string_view().starts_with(TEMP_VARIABLE_PREFIX);
    DMASSERT(!has_temp_prefix || is_variable());
    return has_temp_prefix;
}

inline bool
Symbol::is_function() const noexcept
{
    return type == Type::LIBRARY_FUNCTION || type == Type::PROGRAM_FUNCTION;
}

inline
VarSymbol::VarSymbol(
    const StringSpan name,
    const u32 scope,
    const Type type,
    const Space space,
    const u32 offset,
    const SourceLocation loc,
    const bool is_temp) noexcept
    : Symbol(name, scope, type, loc),
      space(space),
      offset(offset),
      is_temp(is_temp)
{
    DMASSERT(is_variable(), is_temp == has_tempvar_name());
}

inline Symbol::Type
VarSymbol::scope_to_symbol_type(const u32 scope)
{
    return scope == k_global_scope ? Type::GLOBAL_VARIABLE : Type::LOCAL_VARIABLE;
}

inline TempHandleID
VarSymbol::temp_handle() const noexcept
{
    DMASSERT(has_temp_handle() && "Variable symbol has no temp_handle to return");
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
    DMASSERT(is_active());
    return id_;
}

inline
LibFuncSymbol::LibFuncSymbol(const StringSpan name, [[maybe_unused]] u32)
    : Symbol(name, k_libfunc_scope, Symbol::Type::LIBRARY_FUNCTION, SourceLocation::none()) {}

inline
ProgFuncSymbol::ProgFuncSymbol(
    const StringSpan name,
    const u32 scope,
    const u32 address,
    const std::vector<Parameter>& parameter_list,
    const SourceLocation location)
    : Symbol(name, scope, Symbol::Type::PROGRAM_FUNCTION, location),
      address(address),
      parameter_list(parameter_list) {}
} // namespace alpha
#endif // ALPHA_SYMBOLS_HPP
