#include "parser/alpha_symbol_table.hpp"
#include "utils/smart_assert.h"
#include "core/alpha_konstants.hpp"
#include "utils/format_adapter.hpp"
#include <sstream>
#include <initializer_list>
#include <iomanip>

#define STRINGIFY(_x) #_x

namespace Alpha
{
        namespace
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
                DEBUG_ALWAYS_INLINE typename SynonymContainer::const_iterator
                find_insert_position(const SynonymContainer &synonym_symbols, u32 scope)
                {
                        auto symbol_it = synonym_symbols.begin();
                        for (; symbol_it != synonym_symbols.end(); ++symbol_it)
                        {
                                // Same scope and active symbol before insert is error
                                // User of `insert_FUNCTION` should do lookup_first.
                                DEBUG_SMART_ASSERT(symbol_it->scope() != scope || !symbol_it->is_active());
                                if (symbol_it->scope() >= scope) // A bug with a bugged, bug patch lived here :D.
                                        break;                   // This comments pays its respects to Savvidis for teaching defensive programming.
                        }
                        return symbol_it;
                }

                void ensure_scope_slot(auto &symbols_per_scope, u32 scope)
                {
                        if (scope >= symbols_per_scope.size())
                                symbols_per_scope.resize(scope + 1);
                }

        }

        template <typename SymbolKind, typename... ParameterList>
                requires(std::is_same_v<SymbolKind, SymbolTable::Variable> ||
                         std::is_same_v<SymbolKind, SymbolTable::Function>)
        const Symbol *SymbolTable::insert_symbol(const std::string &symbol_name, SymbolType type,
                                                 u32 scope, Location location, ParameterList &&...arg_list)
        {
                DEBUG_SMART_ASSERT(symbol_name.size() > 0);
                auto [symbol_map_it, _] = symbol_map_.try_emplace(symbol_name);
                auto &symbol_name_ref = symbol_map_it->first;
                auto &synonym_symbols = symbol_map_it->second;
                auto symbol_it = find_insert_position(synonym_symbols, scope);
                SymbolKind new_symbol(symbol_name_ref, scope, type, location, std::forward<ParameterList>(arg_list)...);
                Symbol *symbol_ptr = &*synonym_symbols.emplace(symbol_it, std::move(new_symbol));
                DEBUG_SMART_ASSERT(symbol_ptr != nullptr);

                ensure_scope_slot(actives_per_scope_, scope);
                ensure_scope_slot(symbols_per_scope_, scope);
                symbols_per_scope_[scope].push_back(symbol_ptr);
                actives_per_scope_[scope].push_back(symbol_ptr);
                return symbol_ptr;
        }

        // Explicit instantiations for insert_symbol()
        template const Symbol *SymbolTable::insert_symbol<SymbolTable::Variable>(
            const std::string &, SymbolType, u32, Location);

        template const Symbol *SymbolTable::insert_symbol<SymbolTable::Function>(
            const std::string &, SymbolType, u32, Location, std::list<Parameter> &&);

        const Symbol *SymbolTable::insert_global(const std::string &symbol_name, Location location)
        {
                return insert_symbol<Variable>(symbol_name, SymbolType::GLOBAL, k_global_scope, location);
        }

        const Symbol *SymbolTable::insert_formal(const std::string &symbol_name, u32 scope,
                                                 Location location)
        {
                return insert_symbol<Variable>(symbol_name, SymbolType::FORMAL, scope, location);
        }

        const Symbol *SymbolTable::insert_local(const std::string &symbol_name, u32 scope,
                                                Location location)
        {
                return insert_symbol<Variable>(symbol_name, SymbolType::LOCAL, scope, location);
        }

        const Symbol *SymbolTable::insert_function(const std::string &symbol_name, SymbolType type,
                                                   u32 scope, Location location,
                                                   const std::list<Parameter> &argument_list)
        {
                DEBUG_SMART_ASSERT(is_type_function(type));

                return insert_symbol<Function>(symbol_name, type, scope, location, argument_list);
        }

        const Symbol *SymbolTable::insert_anonymous(u32 scope, Location location,
                                                    const std::list<Parameter> &argument_list)
        {
                std::string anonymous_name = fmt_ns::format(
                    "{}{}", k_private_anonymous_prefix, anonymous_counter_++);

                return insert_function(anonymous_name, SymbolType::USERFUNC,
                                       scope, location, std::move(argument_list));
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

        SymbolTable::SymbolTable()
            : anonymous_counter_(0)
        {
                // Load library functions
                for (SymbolName name : k_library_function_names)
                {
                        insert_function(name, SymbolType::LIBFUNC, k_global_scope, Location(0, 0), {});
                        library_function_set_.insert(name);
                }
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
                        if (symbol.scope() < scope)
                                continue;
                        if (symbol.scope() > scope)
                                break;
                        if (symbol.is_active())
                                return &symbol;
                }
                return nullptr;
        }

        const Symbol *SymbolTable::lookup_global(const std::string &symbol_name) const
        {
                // Does `symbol_name` exist ?
                const auto it = symbol_map_.find(symbol_name);
                if (it == symbol_map_.end())
                        return nullptr;

                // Scope lists are sorted; global scope is always at the front if present.
                const auto &scope_list = it->second;
                if (!scope_list.empty() && scope_list.front().scope() == k_global_scope)
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
                        if (symbol_it->scope() <= scope && symbol_it->is_active())
                                return &(*symbol_it);
                return nullptr;
        }
}
