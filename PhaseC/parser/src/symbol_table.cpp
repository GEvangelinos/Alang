#include "parser/symbol_table.hpp"
#include "core/konstants.hpp"  // for k_global_scope, k_private_anony...
#include "parser/_parser_common.hpp"
#include "support/format_adapter.hpp"  // for format, FMT
#include "support/smart_assert.h"      // for DMASSERT
#include <utility>                   // for move, pair, forward

#include "core/string_span.hpp"
#include "parser/semantic_utils.hpp"

namespace alpha
{
inline constexpr std::vector<Parameter> k_empty_parameter_list;

namespace // (Anonymous)
{
    template <typename Key, typename Value>
    [[maybe_unused]] const Key& get_umap_key_ref(std::unordered_map<Key, Value> umap, Key key)
    {
        auto [item_it, inserted] = umap.try_emplace(key);
        DMASSERT(!inserted);                 // This function is meant to be used for existing keys.
        const Key& key_ref = item_it->first; // First item part of item is Key, second is Value.
        return key_ref;
    }

    template <typename SynonymContainer>
    typename SynonymContainer::const_iterator
    find_insert_position(const SynonymContainer& synonym_symbols, u32 scope)
    {
        auto symbol_it = synonym_symbols.begin();
        for (; symbol_it != synonym_symbols.end(); ++symbol_it)
        {
            // Same scope and active symbol before insert is error
            // User of `insert_FUNCTION` should do lookup_first.
            DMASSERT((*symbol_it)->scope != scope || !(*symbol_it)->is_active());
            if ((*symbol_it)->scope >= scope) // A bug with a bugged, bug patch lived here :D.
                break;
            // This comments pays its respects to Savvidis for teaching defensive programming.
        }
        return symbol_it;
    }

    void ensure_scope_slot(auto& symbols_per_scope, u32 scope)
    {
        if (scope >= symbols_per_scope.size())
            symbols_per_scope.resize(scope + 1);
    }
} // namespace

template <typename SymbolKind, typename... Args>
    requires std::is_base_of_v<Symbol, SymbolKind>
const SymbolKind*
SymbolTable::insert_symbol(const StringSpan name, u32 scope, Args&&... args)
{
    DMASSERT(!name.empty());

    const auto symbol_map_it = symbol_map_.try_emplace(name).first;
    const auto& symbol_name_ref = symbol_map_it->first;
    auto& synonym_symbols = symbol_map_it->second;
    const auto symbol_it = find_insert_position(synonym_symbols, scope);
    SymbolPtr symbol_ptr = std::make_unique<SymbolKind>(
        symbol_name_ref,
        scope,
        std::forward<Args>(args)...
    );

    ensure_scope_slot(actives_per_scope_, scope);
    ensure_scope_slot(symbols_per_scope_, scope);
    symbols_per_scope_[scope].push_back(symbol_ptr.get());
    actives_per_scope_[scope].push_back(symbol_ptr.get());

    return DEBUG_REQUIRE_PTR(static_cast<SymbolKind *>(
        synonym_symbols.emplace(symbol_it, std::move(symbol_ptr))->get()
    ));
}

SymbolTable::SymbolTable()
{
    // Load library functions
    for (uf32 i = 0; i < k_library_function_names.size(); i++)
    {
        const StringSpan name = k_library_function_names[i];

        library_function_set_.insert(name);
        (void)insert_symbol<LibFuncSymbol>(name, k_libfunc_scope);
    }
}

// Used for inserting PROGRAM_FUNCTIONS (USER FUNCTIONS)
const ProgFuncSymbol*
SymbolTable::insert_program_function(
    const StringSpan name,
    const u32 scope,
    const LabelID address,
    const std::vector<Parameter>& parameter_list,
    const SourceLocation location)
{
    return insert_symbol<ProgFuncSymbol>(name, scope, address, parameter_list, location);
}

const VarSymbol*
SymbolTable::insert_variable(
    const StringSpan name,
    const u32 scope,
    const VarSymbol::Type type,
    const VarSymbol::Space space,
    const u32 offset,
    const SourceLocation location,
    const bool is_temp)
{
    return insert_symbol<VarSymbol>(name, scope, type, space, offset, location, is_temp);
}

const Symbol*
SymbolTable::lookup_global(const StringSpan name) const
{
    // Does `symbol_name` exist ?
    const auto it = symbol_map_.find(name);
    if (it == symbol_map_.end())
        return nullptr;

    // Scope lists are sorted; global scope is always at the front if present.
    const auto& scope_list = it->second;
    if (!scope_list.empty() && scope_list.front()->scope == k_global_scope)
        return scope_list.front().get();
    return nullptr;
}

const Symbol*
SymbolTable::lookup_nearest(const StringSpan name, const u32 scope) const
{
    // Does `symbol_name` exist ?
    const auto it = symbol_map_.find(name);
    if (it == symbol_map_.end())
        return nullptr;

    // We search from inner to outer scope (end to begin)
    const auto& scope_list = it->second;
    for (auto symbol_it = scope_list.crbegin(); symbol_it != scope_list.crend(); ++symbol_it)
        if (symbol_it->get()->scope <= scope && symbol_it->get()->is_active())
            return symbol_it->get();
    return nullptr;
}

const Symbol*
SymbolTable::lookup_local(const StringSpan name, const u32 scope) const
{
    const auto it = symbol_map_.find(name);
    if (it == symbol_map_.end())
        return nullptr;

    const auto& scope_list = it->second;
    for (auto& symbol_ptr : scope_list)
    {
        if (symbol_ptr->scope < scope)
            continue;
        if (symbol_ptr->scope > scope)
            break;
        if (symbol_ptr->is_active())
            return symbol_ptr.get();
    }
    return nullptr;
}

void
SymbolTable::hide_scope_symbols(const u32 scope) noexcept
{
    DMASSERT(scope > k_global_scope);

    if (scope >= actives_per_scope_.size())
        return; // We don't have symbols at that scope yet. (Empty block at higher scope)

    std::vector<Symbol*>& actives_in_scope = actives_per_scope_[scope];

    for (Symbol* symbol_ptr : actives_in_scope)
    {
        DMASSERT(!!symbol_ptr);
        symbol_ptr->deactivate();
    }

    // At this point, all active symbols in the scope have been deactivated.
    // Since there are no more active symbols in this scope, we can safely
    // clear the vector. (It keeps it capacity, it zeroes its size.)
    actives_in_scope.clear();
}

bool
SymbolTable::is_libfunc_name(const StringSpan name) const
{
    return library_function_set_.contains(name);
}

/// Note: This method is deliberately implemented as a static method of SymbolTable
/// rather than a member of Symbol or Variable.
///
/// Rationale:
/// - The SymbolTable is the sole authority that creates and owns all Symbol instances.
/// - While symbols are accessed externally via const pointers (for safety),
///   this method provides narrow, explicit mutation access for constant propagation.
/// - We intentionally *do not* provide a general mutator or "get mutable symbol" interface.
/// - This function encodes a very specific semantic operation: assigning a const_value
///   to a Variable — and only that.
/// - It is placed here to clearly associate all forms of symbol *construction* and
///   *mutation* within the same logical unit (SymbolTable), ensuring the cost
///   of unsafe behavior stays *close to the code that enables it*.
///
/// Mental Model:
/// - If a user constructs Symbols through SymbolTable, it makes sense for
///   *this same unit* to handle their rare, controlled mutation points.
/// - This tight coupling avoids scattered mutability, const_casts, or unsafe side channels.
/// - We accept that if the user breaks this contract (e.g., passes a symbol from elsewhere),
///   it's *their bug*. This method trusts that the pipeline is well-formed.

void
SymbolTable::detach_const_expr(const VarSymbol* const var_symbol)
{
    DEBUG_REQUIRE_PTR(var_symbol)->const_expr_ = nullptr;
}

// Related method — refer to rationale above
void
SymbolTable::attach_const_expr(
    const VarSymbol* const var_symbol,
    const ConstExpr* const const_expr)
{
    DMASSERT(!!var_symbol, !!const_expr, const_expr->is_const());
    var_symbol->const_expr_ = const_expr;
}

void
SymbolTable::attach_temp_handle(const VarSymbol* const var_symbol, const TempHandleID id)
{
    DMASSERT(!id.is_none());
    DEBUG_REQUIRE_PTR(var_symbol)->temp_binding_.bind(id);
}

TempHandleID
SymbolTable::detach_temp_handle(const VarSymbol* const var_symbol)
{
    return DEBUG_REQUIRE_PTR(var_symbol)->temp_binding_.release();
}

void
SymbolTable::mark_as_initialized(const VarSymbol* const var_symbol)
{
    DEBUG_REQUIRE_PTR(var_symbol)->is_initialized_.raise();
}
} // namespace alpha
