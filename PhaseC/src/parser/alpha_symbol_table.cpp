#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_konstants.hpp"  // for k_global_scope, k_private_anony...
#include "parser/_parser_common.hpp" // for Parameter
#include "utils/format_adapter.hpp"  // for format, fmt_ns
#include "utils/smart_assert.h"      // for DEBUG_SMART_ASSERT
#include <initializer_list>          // for initializer_list
#include <utility>                   // for move, pair, forward

namespace Alpha
{
static const std::list<Parameter> k_empty_param_list;
// Explicit instantiations for insert_symbol()
template const Symbol *SymbolTable::insert_symbol<Variable>(Variable &&);
template const Symbol *SymbolTable::insert_symbol<Function>(Function &&);

namespace // (Anonymous)
{
template <typename Key, typename Value>
const Key &get_umap_key_ref(std::unordered_map<Key, Value> umap, Key key)
{
        auto [item_it, inserted] = umap.try_emplace(key);
        DEBUG_SMART_ASSERT(!inserted);       // This function is meant to be used for existing keys.
        const Key &key_ref = item_it->first; // First item part of item is Key, second is Value.
        return key_ref;
}

template <typename SynonymContainer>
DEBUG_ALWAYS_INLINE typename SynonymContainer::const_iterator find_insert_position(
    const SynonymContainer &synonym_symbols, u32 scope)
{
        auto symbol_it = synonym_symbols.begin();
        for (; symbol_it != synonym_symbols.end(); ++symbol_it)
        {
                // Same scope and active symbol before insert is error
                // User of `insert_FUNCTION` should do lookup_first.
                DEBUG_SMART_ASSERT(symbol_it->scope != scope || !symbol_it->is_active());
                if (symbol_it->scope >= scope) // A bug with a bugged, bug patch lived here :D.
                        break; // This comments pays its respects to Savvidis for teaching defensive
                               // programming.
        }
        return symbol_it;
}

void ensure_scope_slot(auto &symbols_per_scope, u32 scope)
{
        if (scope >= symbols_per_scope.size())
                symbols_per_scope.resize(scope + 1);
}

} // namespace

std::string_view Symbol::type_to_string() const noexcept
{
        switch (type)
        {
        case Symbol::Type::LIBRARY_FUNCTION:
                return "LIBRARY_FUNCTION";
        case Symbol::Type::PROGRAM_FUNCTION:
                return "PROGRAM_FUNCTION";
        case Symbol::Type::VARIABLE:
                return "VARIABLE";
        }
        UNREACHABLE("Some field of Symbol::Type is not registred");
}

SymbolTable::SymbolTable()
{
        // Load library functions
        for (SymbolName name : k_library_function_names)
        {
                insert_function(name, Symbol::Type::LIBRARY_FUNCTION, k_global_scope,
                                k_empty_param_list, k_no_location);
                library_function_set_.insert(name);
        }
}

const Symbol *SymbolTable::insert_function(const std::string &symbol_name, Symbol::Type type,
                                           u32 scope, const std::list<Parameter> &parameter_list,
                                           Location location)
{
        DEBUG_SMART_ASSERT(type == Symbol::Type::LIBRARY_FUNCTION ||
                           type == Symbol::Type::PROGRAM_FUNCTION);
        return insert_symbol<Function>(
            Function(symbol_name, scope, type, parameter_list, location));
}

const Symbol *SymbolTable::insert_anonymous(u32 scope, Location location,
                                            const std::list<Parameter> &parameter_list)
{
        std::string anonymous_name =
            fmt_ns::format("{}{}", k_private_anonymous_prefix, anonymous_counter_++);

        return insert_function(anonymous_name, Symbol::Type::PROGRAM_FUNCTION, scope,
                               parameter_list, location);
}
const Symbol *SymbolTable::insert_variable(const std::string &symbol_name, u32 scope, u32 offset,
                                           Location location)
{
        return insert_symbol<Variable>(
            Variable(symbol_name, scope, Variable::ScopeSpace::PROGRAM_VAR, offset, location));
}

const Symbol *SymbolTable::lookup_global(const std::string &symbol_name) const
{
        // Does `symbol_name` exist ?
        const auto it = symbol_map_.find(symbol_name);
        if (it == symbol_map_.end())
                return nullptr;

        // Scope lists are sorted; global scope is always at the front if present.
        const auto &scope_list = it->second;
        if (!scope_list.empty() && scope_list.front().scope == k_global_scope)
                return &scope_list.front();
        return nullptr;
}

const Symbol *SymbolTable::lookup_chain(const std::string &symbol_name, u32 scope) const
{
        // Does `symbol_name` exist ?
        const auto it = symbol_map_.find(symbol_name);
        if (it == symbol_map_.end())
                return nullptr;

        // We search from inner to outer scope (end to begin)
        const auto &scope_list = it->second;
        for (auto symbol_it = scope_list.crbegin(); symbol_it != scope_list.crend(); ++symbol_it)
                if (symbol_it->scope <= scope && symbol_it->is_active())
                        return &(*symbol_it);
        return nullptr;
}

const Symbol *SymbolTable::lookup_local(const std::string &symbol_name, u32 scope) const
{
        // Does `symbol_name` exist ?
        const auto it = symbol_map_.find(symbol_name);
        if (it == symbol_map_.end())
                return nullptr;

        const auto &scope_list = it->second;
        for (const auto &symbol : scope_list)
        {
                if (symbol.scope < scope)
                        continue;
                if (symbol.scope > scope)
                        break;
                if (symbol.is_active())
                        return &symbol;
        }
        return nullptr;
}

void SymbolTable::hide_scope_symbols(u32 scope) noexcept
{
        DEBUG_SMART_ASSERT(scope > k_global_scope);

        if (scope >= actives_per_scope_.size())
                return; // We don't have symbols at that scope yet. (Empty block at higher scope)

        std::vector<Symbol *> &actives_in_scope = actives_per_scope_[scope];

        for (Symbol *symbol_ptr : actives_in_scope)
        {
                DEBUG_SMART_ASSERT(symbol_ptr != nullptr);
                symbol_ptr->deactivate();
        }

        // At this point, all active symbols in the scope have been deactivated.
        // Since there are no more active symbols in this scope, we can safely
        // clear the vector. (It keeps it capacity, it zeroes its size.)
        actives_in_scope.clear();
}

bool SymbolTable::is_lib_function(const std::string &symbol_name) const
{
        return library_function_set_.contains(symbol_name);
}

template <typename SymbolKind>
        requires(std::is_same_v<SymbolKind, Variable> || std::is_same_v<SymbolKind, Function>)
const Symbol *SymbolTable::insert_symbol(SymbolKind &&new_symbol)
{
        DEBUG_SMART_ASSERT(symbol_name.size() > 0);
        auto [symbol_map_it, _] = symbol_map_.try_emplace(symbol_name);
        auto &symbol_name_ref = symbol_map_it->first;
        auto &synonym_symbols = symbol_map_it->second;
        auto symbol_it = find_insert_position(synonym_symbols, scope);

        Symbol *symbol_ptr = &*synonym_symbols.emplace(symbol_it, std::move(new_symbol));
        DEBUG_SMART_ASSERT(symbol_ptr != nullptr);

        ensure_scope_slot(actives_per_scope_, scope);
        ensure_scope_slot(symbols_per_scope_, scope);
        symbols_per_scope_[scope].push_back(symbol_ptr);
        actives_per_scope_[scope].push_back(symbol_ptr);
        return symbol_ptr;
}
} // namespace Alpha
