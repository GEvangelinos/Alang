#include "parser/symbol_table.hpp"
#include "core/konstants.hpp"  // for k_global_scope, k_private_anony...
#include "parser/_parser_common.hpp"
#include "utils/format_adapter.hpp"  // for format, FMT
#include "utils/smart_assert.h"      // for DEBUG_SMART_ASSERT
#include <utility>                   // for move, pair, forward
#include "parser/RELOCATE_backpatcher.hpp"
#include "parser/semantic_utils.hpp"

namespace Alpha
{
static const std::list<Parameter> k_empty_parameter_list;

namespace // (Anonymous)
{
    template<typename Key, typename Value>
    [[maybe_unused]] const Key &get_umap_key_ref(std::unordered_map<Key, Value> umap, Key key)
    {
        auto [item_it, inserted] = umap.try_emplace(key);
        DEBUG_SMART_ASSERT(!inserted);       // This function is meant to be used for existing keys.
        const Key &key_ref = item_it->first; // First item part of item is Key, second is Value.
        return key_ref;
    }

    template<typename SynonymContainer>
    DEBUG_ALWAYS_INLINE typename SynonymContainer::const_iterator
    find_insert_position(const SynonymContainer &synonym_symbols, u32 scope)
    {
        auto symbol_it = synonym_symbols.begin();
        for (; symbol_it != synonym_symbols.end(); ++symbol_it)
        {
            // Same scope and active symbol before insert is error
            // User of `insert_FUNCTION` should do lookup_first.
            DEBUG_SMART_ASSERT((*symbol_it)->scope != scope || !(*symbol_it)->is_active());
            if ((*symbol_it)->scope >= scope) // A bug with a bugged, bug patch lived here :D.
                break;
            // This comments pays its respects to Savvidis for teaching defensive programming.
        }
        return symbol_it;
    }

    void ensure_scope_slot(auto &symbols_per_scope, u32 scope)
    {
        if (scope >= symbols_per_scope.size())
            symbols_per_scope.resize(scope + 1);
    }
} // namespace

template<typename SymbolKind, typename... Args>
    requires std::is_same_v<SymbolKind, Variable> || std::is_same_v<SymbolKind, Function>
DEBUG_ALWAYS_INLINE const SymbolKind *
SymbolTable::insert_symbol(
    const std::string &name,
    u32 scope,
    Args &&... args)
{
    DEBUG_SMART_ASSERT(name.size() > 0);

    const auto symbol_map_it = symbol_map_.try_emplace(name).first;
    const auto &symbol_name_ref = symbol_map_it->first;
    auto &synonym_symbols = symbol_map_it->second;
    const auto symbol_it = find_insert_position(synonym_symbols, scope);
    SymbolPtr symbol_ptr = std::make_unique<SymbolKind>(
        symbol_name_ref, scope, std::forward<Args>(args)...);

    ensure_scope_slot(actives_per_scope_, scope);
    ensure_scope_slot(symbols_per_scope_, scope);
    symbols_per_scope_[scope].push_back(symbol_ptr.get());
    actives_per_scope_[scope].push_back(symbol_ptr.get());

    return static_cast<SymbolKind *>(
        synonym_symbols.emplace(symbol_it, std::move(symbol_ptr))->get());
}

// Explicit instantiations for insert_symbol()
template const Function *
SymbolTable::insert_symbol<Function>(
    const std::string &, u32, Function::Type &&, u32 &&, const std::list<Parameter> &,
    SourceLocation &&);

template const Variable *
SymbolTable::insert_symbol<Variable>(
    const std::string &, u32, Variable::Type &&, Variable::Space &&, u32 &&, SourceLocation &&);

SymbolTable::SymbolTable()
{
    // Load library functions
    for (uf32 i = 0; i < k_library_function_names.size(); i++)
    {
        const std::string &name = k_library_function_names[i];
        const auto function_address = i;

        const Function *const function_symbol = insert_symbol<Function>(
            name,
            k_global_scope,
            Symbol::Type::LIBRARY_FUNCTION,
            function_address,
            k_empty_parameter_list,
            k_no_loc);
        library_function_set_.insert(name);

        Backpatcher::set_function_local_variable_count(
            function_symbol,
            k_libfunc_local_variable_count);
    }
}

// Used for inserting PROGRAM_FUNCTIONS (USER FUNCTIONS)
const Function *
SymbolTable::insert_function(
    const std::string &name,
    const u32 scope,
    const u32 address,
    const std::list<Parameter> &parameter_list,
    const SourceLocation location)
{
    return insert_symbol<Function>(
        name, scope, Symbol::Type::PROGRAM_FUNCTION, address, parameter_list, location);
}

const Variable *
SymbolTable::insert_variable(
    const std::string &name,
    const u32 scope,
    const Variable::Type type,
    const Variable::Space space,
    const u32 offset,
    const SourceLocation location)
{
    return insert_symbol<Variable>(name, scope, type, space, offset, location);
}

const Symbol *
SymbolTable::lookup_global(const std::string &name) const
{
    // Does `symbol_name` exist ?
    const auto it = symbol_map_.find(name);
    if (it == symbol_map_.end())
        return nullptr;

    // Scope lists are sorted; global scope is always at the front if present.
    const auto &scope_list = it->second;
    if (!scope_list.empty() && scope_list.front()->scope == k_global_scope)
        return scope_list.front().get();
    return nullptr;
}

const Symbol *
SymbolTable::lookup_chain(const std::string &name, const u32 scope) const
{
    // Does `symbol_name` exist ?
    const auto it = symbol_map_.find(name);
    if (it == symbol_map_.end())
        return nullptr;

    // We search from inner to outer scope (end to begin)
    const auto &scope_list = it->second;
    for (auto symbol_it = scope_list.crbegin(); symbol_it != scope_list.crend(); ++symbol_it)
        if (symbol_it->get()->scope <= scope && symbol_it->get()->is_active())
            return symbol_it->get();
    return nullptr;
}

const Symbol *
SymbolTable::lookup_local(const std::string &symbol_name, const u32 scope) const
{
    const auto it = symbol_map_.find(symbol_name);
    if (it == symbol_map_.end())
        return nullptr;

    const auto &scope_list = it->second;
    for (auto &symbol_ptr: scope_list)
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
    DEBUG_SMART_ASSERT(scope > k_global_scope);

    if (scope >= actives_per_scope_.size())
        return; // We don't have symbols at that scope yet. (Empty block at higher scope)

    std::vector<Symbol *> &actives_in_scope = actives_per_scope_[scope];

    for (Symbol *symbol_ptr: actives_in_scope)
    {
        DEBUG_SMART_ASSERT(!!symbol_ptr);
        symbol_ptr->deactivate();
    }

    // At this point, all active symbols in the scope have been deactivated.
    // Since there are no more active symbols in this scope, we can safely
    // clear the vector. (It keeps it capacity, it zeroes its size.)
    actives_in_scope.clear();
}

bool
SymbolTable::is_lib_function(const std::string &name) const
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
SymbolTable::override_clear_const_value(const Variable *var)
{
    const_cast<Variable *>(var)->const_expr_ = nullptr;
}

// Related method — refer to rationale above
void
SymbolTable::override_set_const_value(const Variable *var, const ConstExpr *const const_expr)
{
    DEBUG_SMART_ASSERT(!!const_expr, SemUtils::is_const_expr(const_expr));
    const_cast<Variable *>(var)->const_expr_ = const_expr;
}
} // namespace Alpha
