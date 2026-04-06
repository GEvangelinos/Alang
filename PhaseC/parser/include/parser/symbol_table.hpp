#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include "_parser_common.hpp"
#include "core/source_location.hpp" // for Location
#include "core/numeric_types.hpp"    // for u32
#include <list>             // for list
#include <string>           // for basic_string, string, hash, opera...
#include <type_traits>      // for is_same_v
#include <unordered_map>    // for unordered_map
#include <unordered_set>    // for unordered_set
#include <vector>           // for vector
#include <array>
#include <memory>

#include "core/string_span.hpp"
#include "parser/symbols.hpp"

namespace alpha
{
// Classes defined here:
class SymbolTable; // IWYU pragma: keep

const std::array<StringSpan, 12> k_library_function_names = {
    StringSpan::from_literal("print"),
    StringSpan::from_literal("input"),
    StringSpan::from_literal("objectmemberkeys"),
    StringSpan::from_literal("objecttotalmembers"),
    StringSpan::from_literal("objectcopy"),
    StringSpan::from_literal("totalarguments"),
    StringSpan::from_literal("argument"),
    StringSpan::from_literal("typeof"),
    StringSpan::from_literal("strtonum"),
    StringSpan::from_literal("sqrt"),
    StringSpan::from_literal("cos"),
    StringSpan::from_literal("sin")
};

class SymbolTable : private Immobile
{
public:
    using SymbolName = alpha::StringSpan;
    using SymbolPtr = std::unique_ptr<Symbol>;
    using SymbolMap = std::unordered_map<SymbolName, std::list<SymbolPtr>>;

    SymbolTable();
    ~SymbolTable() = default;

    const ProgFuncSymbol* insert_program_function(
        StringSpan name,
        u32 scope,
        u32 address,
        const std::vector<Parameter>& parameter_list,
        SourceLocation location);

    const VarSymbol* insert_variable(
        StringSpan name,
        u32 scope,
        VarSymbol::Type type,
        VarSymbol::Space space,
        u32 offset,
        SourceLocation location);

    [[nodiscard]] const Symbol* lookup_global(StringSpan name) const;
    [[nodiscard]] const Symbol* lookup_nearest(StringSpan name, u32 scope) const;
    [[nodiscard]] const Symbol* lookup_local(StringSpan name, u32 scope) const;

    void hide_scope_symbols(u32 scope) noexcept;
    [[nodiscard]] bool is_libfunc_name(StringSpan name) const;
    [[nodiscard]] const auto& symbols_per_scope() const { return symbols_per_scope_; }

    /// The following methods provide controlled overrides for VarSymbols.
    /// See SymbolTable.cpp for detailed rationale.
    static void detach_const_expr(const VarSymbol* var_symbol);
    static void attach_const_expr(const VarSymbol* var_symbol, const ConstExpr* const_expr);
    static void attach_temp_handle(const VarSymbol* var_symbol, TempHandleID id);
    [[nodiscard]] static TempHandleID detach_temp_handle(const VarSymbol* var_symbol);

private:
    SymbolMap symbol_map_;
    // Are inserted in sorted order based on symbol insertion.
    std::vector<std::vector<const Symbol*>> symbols_per_scope_;
    // Are inserted in sorted order based on symbol insertion.
    std::vector<std::vector<Symbol*>> actives_per_scope_;
    std::unordered_set<SymbolName> library_function_set_;

    template <typename SymbolKind, typename... Args>
        requires std::is_base_of_v<Symbol, SymbolKind>
    [[nodiscard]] const SymbolKind* insert_symbol(StringSpan name, u32 scope, Args&&... args);
};
} // namespace alpha
#endif // SYMBOL_TABLE_HPP
