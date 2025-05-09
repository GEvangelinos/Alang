#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_konstants.hpp"  // for k_global_scope, k_private_anony...
#include "parser/_parser_common.hpp" // for Parameter
#include "utils/format_adapter.hpp"  // for format, FMT
#include "utils/smart_assert.h"      // for DEBUG_SMART_ASSERT
#include <initializer_list>          // for initializer_list
#include <utility>                   // for move, pair, forward

namespace Alpha
{
        static const std::list<Parameter> k_empty_parameter_list;

        namespace // (Anonymous)
        {
                template <typename Key, typename Value>
                [[maybe_unused]] const Key &get_umap_key_ref(std::unordered_map<Key, Value> umap, Key key)
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
                                DEBUG_SMART_ASSERT(symbol_it->scope != scope || !symbol_it->is_active());
                                if (symbol_it->scope >= scope) // A bug with a bugged, bug patch lived here :D.
                                        break;                 // This comments pays its respects to
                                                               // Savvidis for teaching defensive programming.
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

        template <typename SymbolKind, typename... Args>
                requires std::is_same_v<SymbolKind, Variable> ||
                         std::is_same_v<SymbolKind, Function>
        DEBUG_ALWAYS_INLINE const Symbol *SymbolTable::insert_symbol(
            const std::string &name,
            u32 scope,
            Args &&...args)
        {
                DEBUG_SMART_ASSERT(name.size() > 0);

                auto symbol_map_it = symbol_map_.try_emplace(name).first;
                auto &symbol_name_ref = symbol_map_it->first;
                auto &synonym_symbols = symbol_map_it->second;
                auto symbol_it = find_insert_position(synonym_symbols, scope);
                SymbolKind new_symbol(symbol_name_ref, scope, std::forward<Args>(args)...);
                Symbol *symbol_ptr = &*synonym_symbols.emplace(symbol_it, std::move(new_symbol));
                DEBUG_SMART_ASSERT(symbol_ptr != nullptr);

                ensure_scope_slot(actives_per_scope_, scope);
                ensure_scope_slot(symbols_per_scope_, scope);
                symbols_per_scope_[scope].push_back(symbol_ptr);
                actives_per_scope_[scope].push_back(symbol_ptr);
                return symbol_ptr;
        }

        // Explicit instantiations for insert_symbol()
        template const Symbol *SymbolTable::insert_symbol<Function>(
            const std::string &, u32, Symbol::Type &&, const std::list<Parameter> &, Location &&);

        template const Symbol *SymbolTable::insert_symbol<Variable>(
            const std::string &, u32, Variable::Space &&, u32 &&, Location &&);

        template <typename Self>
        DEBUG_ALWAYS_INLINE auto SymbolTable::lookup_local_impl(Self &self, const std::string &name, u32 scope)
            -> decltype(&self.symbol_map_.begin()->second.front())
        {
                auto it = self.symbol_map_.find(name);
                if (it == self.symbol_map_.end())
                        return nullptr;

                auto &scope_list = it->second;
                for (auto &symbol : scope_list)
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

        template Symbol *
        SymbolTable::lookup_local_impl(SymbolTable &self, const std::string &name, u32 scope);

        template const Symbol *
        SymbolTable::lookup_local_impl(const SymbolTable &self, const std::string &name, u32 scope);

        SymbolTable::SymbolTable()
        {
                // Load library functions
                for (SymbolName name : k_library_function_names)
                {
                        insert_symbol<Function>(
                            name,
                            k_global_scope,
                            Symbol::Type::LIBRARY_FUNCTION,
                            k_empty_parameter_list,
                            k_no_location);
                        library_function_set_.insert(name);
                }
        }

        // Used for inserting PROGRAM_FUNCTIONS (USER FUNCNTIONS)
        const Symbol *SymbolTable::insert_function(
            const std::string &name,
            u32 scope,
            const std::list<Parameter> &parameter_list,
            Location location)
        {
                return insert_symbol<Function>(
                    name, scope, Symbol::Type::PROGRAM_FUNCTION, parameter_list, location);
        }

        const Symbol *SymbolTable::insert_variable(
            const std::string &name,
            u32 scope,
            Variable::Space space,
            u32 offset,
            Location location)
        {
                return insert_symbol<Variable>(name, scope, space, offset, location);
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
                return lookup_local_impl(*this, symbol_name, scope);
        }

        void SymbolTable::finalize_function_locals(
            const std::string &name,
            u32 scope,
            u32 local_variables)
        {
                Symbol *found_symbol = lookup_local_impl(*this, name, scope);
                if (found_symbol == nullptr)
                        throw std::logic_error("Function tracked by ParseCtx is not found in SymbolTable");

                DEBUG_SMART_ASSERT(                                      //
                    (dynamic_cast<Function *>(found_symbol) != nullptr), //
                    (found_symbol->is_function())                        //
                );

                static_cast<Function *>(found_symbol)->local_variables.set(local_variables);
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

} // namespace Alpha
