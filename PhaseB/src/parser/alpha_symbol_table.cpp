#include "parser/alpha_symbol_table.hpp"
#include "misc/sanity_assert.h"
#include "core/alpha_konstants.hpp"
#include <format>
#include <sstream>
#include <initializer_list>

#define STRINGIFY(_x) #_x

namespace Alpha
{
        namespace Detail
        {
                template <typename Key, typename Value>
                const Key &get_umap_key_ref(std::unordered_map<Key, Value> umap, Key key)
                {
                        auto [item_it, inserted] = umap.try_emplace(key);
                        SANITY_ASSERT_FALSE(inserted);       // This function is meant to be used for existing keys.
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
                                SANITY_ASSERT_FALSE(symbol_it->scope() == scope &&
                                                    symbol_it->is_active());
                                if (symbol_it->scope() > scope)
                                        break;
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
                SANITY_ASSERT_GT(symbol_name.size(), 0);
                auto [symbol_map_it, _] = symbol_map_.try_emplace(symbol_name);
                auto &symbol_name_ref = symbol_map_it->first;
                auto &synonym_symbols = symbol_map_it->second;
                auto symbol_it = Detail::find_insert_position(synonym_symbols, scope);
                SymbolKind new_symbol(symbol_name_ref, scope, type, location, std::forward<ParameterList>(arg_list)...);
                const Symbol *symbol_ptr = &*synonym_symbols.emplace(symbol_it, std::move(new_symbol));
                Detail::ensure_scope_slot(symbols_per_scope_, scope);
                symbols_per_scope_[scope].push_back(symbol_ptr);
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
                SANITY_ASSERT_TRUE(is_type_function(type));

                return insert_symbol<Function>(symbol_name, type, scope, location, argument_list);
        }

        const Symbol *SymbolTable::insert_anonymous(u32 scope, Location location,
                                                    const std::list<Parameter> &argument_list)
        {
                std::string anonymous_name = std::format(
                    "{}{}", k_private_anonymous_prefix, anonymous_counter_++);

                return insert_function(anonymous_name, SymbolType::USERFUNC,
                                       scope, location, std::move(argument_list));
        }

        // SANITY code here catches 2 nasty bugs:
        // 1) An active symbol appearing *after* an inactive one in the same scope.
        //    - Active symbols must always appear *before* inactive ones at the same scope.
        // 2) More than one active symbol in the same scope.
        //    - This shouldn't happen unless lookups are happening *before* insertions.
        // 3) Yeah, I know... this could be 3 lines of elegance...
        //    But it already caught two nasty bugs -- so it's staying!
        void SymbolTable::hide_scope_symbols(u32 scope) noexcept
        {
                for (auto &synonym_symbols : symbol_map_)
                {
                        SANITY_CODE(u32 deactivated_symbols = 0);
                        auto &symbol_list = synonym_symbols.second;
                        for (auto &symbol : symbol_list)
                        {
                                if (scope < symbol.scope())
                                        continue;
                                if (scope > symbol.scope()) // After deactivation we should exit from here
                                        break;
                                SANITY_ASSERT_FALSE(deactivated_symbols > 0 && symbol.is_active());
                                symbol.deactivate(); // Check if already deactivated is redundant.
                                SANITY_CODE(++deactivated_symbols);
                        }
                }
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
