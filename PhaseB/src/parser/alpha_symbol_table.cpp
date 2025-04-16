#include "parser/alpha_symbol_table.hpp"
#include "misc/sanity_assert.h"
#include "core/alpha_konstants.hpp"
#include <format>
#include <sstream>
#include <initializer_list>

#define STRINGIFY(_x) #_x

namespace Alpha
{
        std::initializer_list<SymbolTable::SymbolName> library_function_names = {
            "print",
            "input",
            "objectmemberkeys",
            "objecttotalmembers",
            "objectcopy",
            "totalarguments",
            "argument",
            "typeof",
            "strtonum",
            "sqrt",
            "cos",
            "sin"};

        // Can I use here in SymbolTable::SymbolTable() // constructor?

        template <typename T>
        inline constexpr bool always_false = false;

        template <typename SymbolKind, typename... ArgumentList>
        void SymbolTable::insert_symbol(const std::string &symbol_name, SymbolType type,
                                        u32 scope, CodeLocation location, ArgumentList &&...arg_list)
        {
                if constexpr (std::is_same_v<SymbolKind, Variable>)
                        static_assert(sizeof...(arg_list) == 0, "Variables do not take arguments");
                else if constexpr (std::is_same_v<SymbolKind, Function>)
                        static_assert(sizeof...(arg_list) == 1, "Functions take exactly 1 argument list");
                else
                        static_assert(always_false<SymbolKind>, "insert_symbol() accepts only Variable or Function");

                SANITY_ASSERT_GT(symbol_name.size(), 0);

                // All symbol with same symbol_name (on different scopes).
                std::list<Symbol> &synonym_symbols = symbol_map_[symbol_name];

                auto it = synonym_symbols.begin();
                for (; it != synonym_symbols.end(); ++it)
                {
                        // Same scope and active symbol before insert is error
                        // User of `insert_variable` should do loopup_first.
                        SANITY_ASSERT_FALSE(scope == it->scope() && it->is_active());
                        if (scope < it->scope())
                                break;
                }
                synonym_symbols.emplace(it, SymbolKind(scope, type, location,
                                                       std::forward<ArgumentList>(arg_list)...));
        }

        // Explicit instantiations for insert_symbol()
        template void SymbolTable::insert_symbol<SymbolTable::Variable>(
            const std::string &, SymbolType, u32, CodeLocation);

        template void SymbolTable::insert_symbol<SymbolTable::Function>(
            const std::string &, SymbolType, u32, CodeLocation, std::list<Parameter> &&);

        void SymbolTable::insert_variable(const std::string &symbol_name, SymbolType type,
                                          u32 scope, CodeLocation location)
        {
                SANITY_ASSERT_TRUE(is_type_varible(type));
                insert_symbol<Variable>(symbol_name, type, scope, location);
        }

        void SymbolTable::insert_function(const std::string &symbol_name, SymbolType type,
                                          u32 scope, CodeLocation location,
                                          std::list<Parameter> argument_list)
        {
                SANITY_ASSERT_TRUE(is_type_function(type));
                insert_symbol<Function>(symbol_name, type, scope, location, std::move(argument_list));
        }

        void SymbolTable::insert_anonymous(u32 scope, CodeLocation location,
                                           std::list<Parameter> argument_list)
        {
                std::string anonymous_name = std::format(
                    "{}{}", k_anonymous_function_prefix, anonymous_counter_++);

                insert_function(anonymous_name, SymbolType::USERFUNC,
                                scope, location, std::move(argument_list));
        }

        // SANITY code here catches 2 nasty bugs:
        // 1) An active symbol appearing *after* an inactive one in the same scope.
        //    - Active symbols must always appear *before* inactive ones at the same scope.
        // 2) More than one active symbol in the same scope.
        //    - This shouldn't happen unless lookups are happening *before* insertions.
        // 3) Yeah, I know... this could be 3 lines of elegance...
        //    But it already caught two nasty bugs -- so it's staying!
        void SymbolTable::hide_scope_symbols(u32 scope)
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
                                SANITY_ASSERT_FALSE(deactivated_symbols == 0 && symbol.is_active());
                                symbol.deactivate(); // Check if already deactivates is redundant.
                                SANITY_CODE(++deactivated_symbols);
                        }
                }
        }

        bool SymbolTable::is_lib_function(const std::string &symbol_name)
        {
                return library_function_set_.contains(symbol_name);
        }

        SymbolTable::SymbolTable()
            : anonymous_counter_(0)
        {
                // Load library functions
                for (SymbolName name : library_function_names)
                {
                        insert_function(name, SymbolType::LIBFUNC, 0, CodeLocation(0, 0));
                        library_function_set_.insert(name);
                }
        }

        const Symbol *SymbolTable::lookup_local(const std::string &symbol_name, u32 scope) const noexcept
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

        const Symbol *SymbolTable::lookup_global(const std::string &symbol_name) const noexcept
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

        const Symbol *SymbolTable::lookup_chain(const std::string &symbol_name, u32 scope) const noexcept
        {
                // Does `symbol_name` exist ?
                const auto it = symbol_map_.find(symbol_name);
                if (it == symbol_map_.end())
                        return nullptr;

                const auto &scope_list = it->second;
                for (auto symbol_it = scope_list.crbegin(); symbol_it != scope_list.crend(); ++symbol_it)
                        if (symbol_it->scope() <= scope && symbol_it->is_active())
                                return &(*symbol_it);
                return nullptr;
        }
}
