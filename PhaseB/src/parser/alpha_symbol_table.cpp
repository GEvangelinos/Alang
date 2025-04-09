#include "parser/alpha_symbol_table.hpp"
#include "misc/sanity_assert.h"
#include "core/alpha_konstants.hpp"
#include <format>

namespace Alpha
{
        template <typename T>
        inline constexpr bool always_false = false;

        template <typename SymbolKind, typename... ArgumentList>
        void SymbolTable::insert_symbol(const std::string &name, SymbolType type,
                                        u32 scope, CodeLocation location, ArgumentList &&...arg_list)
        {
                if constexpr (std::is_same_v<SymbolKind, Variable>)
                        static_assert(sizeof...(arg_list) == 0, "Variables do not take arguments");
                else if constexpr (std::is_same_v<SymbolKind, Function>)
                        static_assert(sizeof...(arg_list) == 1, "Functions take exactly 1 argument list");
                else
                        static_assert(always_false<SymbolKind>, "insert_symbol() accepts only Variable or Function");

                SANITY_ASSERT_GT(name.size(), 0);

                SymbolList &synonym_symbols = symbol_map_[name]; // All symbols with same name.

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

        void SymbolTable::insert_variable(const std::string &name, SymbolType type,
                                          u32 scope, CodeLocation location)
        {
                SANITY_ASSERT_TRUE(is_type_varible(type));
                insert_symbol<Variable>(name, type, scope, location);
        }

        void SymbolTable::insert_function(const std::string &name, SymbolType type,
                                          u32 scope, CodeLocation location,
                                          std::list<Parameter> &&argument_list)
        {
                SANITY_ASSERT_TRUE(is_type_function(type));
                insert_symbol<Function>(name, type, scope, location, std::move(argument_list));
        }

        void SymbolTable::insert_anonymous(u32 scope, CodeLocation location,
                                           std::list<Parameter> &&argument_list)
        {
                std::string anonymous_name = std::format(
                    "{}{}", k_anonymous_function_prefix, anonymous_counter_.post_inc());

                insert_function(anonymous_name, SymbolType::USERFUNC,
                                scope, location, std::move(argument_list));
        }
}