#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include "_parser_common.hpp"      // for Parameter
#include "core/alpha_location.hpp" // for Location
#include "core/alpha_macros.hpp"   // for DEBUG_ALWAYS_INLINE
#include "core/alpha_types.hpp"    // for u32
#include "utils/smart_assert.h"    //
#include <initializer_list>        // for initializer_list
#include <list>                    // for list
#include <string>                  // for basic_string, string, hash, opera...
#include <string_view>             // for string_view
#include <type_traits>             // for is_same_v
#include <unordered_map>           // for unordered_map
#include <unordered_set>           // for unordered_set
#include <vector>                  // for vector

namespace Alpha
{
        // Classes defined here:
        class Symbol;      // IWYU pragma: keep
        class SymbolTable; // IWYU pragma: keep
        class Variable;    // IYU pragma: keep
        class Function;    // IWYU pragma: keep

        const std::initializer_list<std::string> k_library_function_names = {
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

        class Symbol // Lean version (it doesn't contain name, Symbol Table keeps that as its key).
        {
        public:
                enum class Type
                {
                        LIBRARY_FUNCTION,
                        PROGRAM_FUNCTION,
                        VARIABLE,
                };

                const std::string &name;
                const u32 scope;
                const Type type;
                const Location location;

                std::string_view type_to_string() const noexcept;

                DEBUG_ALWAYS_INLINE bool is_variable() const noexcept { return type == Type::VARIABLE; }
                DEBUG_ALWAYS_INLINE bool is_function() const noexcept { return !is_variable(); }
                DEBUG_ALWAYS_INLINE bool is_active() const noexcept { return is_active_; }

        protected:
                Symbol(const std::string &name, u32 scope, Type type, Location location) noexcept
                    : name(name), scope(scope), type(type), location(location)
                {
                }

        private:
                bool is_active_ = true;

                DEBUG_ALWAYS_INLINE void activate() noexcept { is_active_ = true; }
                DEBUG_ALWAYS_INLINE void deactivate() noexcept { is_active_ = false; }

                friend class SymbolTable;
        };

        class Variable : public Symbol
        {
        public:
                enum class Space
                {
                        PROGRAM_VAR,
                        FUNCTION_LOCAL,
                        FORMAL_ARGUMENT,
                };

                const Space space;
                const u32 offset;

                Variable(const std::string &name, u32 scope, Space space, u32 offset, Location location)
                    : Symbol(name, scope, Symbol::Type::VARIABLE, location), space(space), offset(offset)
                {
                }
        };

        class Function : public Symbol
        {
        public:
                const std::list<Parameter> parameter_list;
                Function(const std::string &name, const u32 scope, const Symbol::Type type,
                         const std::list<Parameter> &parameter_list, const Location location)
                    : Symbol(name, scope, type, location), parameter_list(parameter_list)
                {
                        DEBUG_SMART_ASSERT(type != Symbol::Type::VARIABLE);
                }
        };

        class SymbolTable
        {
        public:
                using SymbolName = std::string;
                using SymbolMap = std::unordered_map<SymbolName, std::list<Symbol>>;

                SymbolTable();
                ~SymbolTable() = default;
                SymbolTable(const SymbolTable &) = delete;
                SymbolTable(const SymbolTable &&) = delete;
                SymbolTable &operator=(const SymbolTable &) = delete;
                SymbolTable &operator=(const SymbolTable &&) = delete;

                const Symbol *insert_function(
                    const std::string &name,
                    Symbol::Type type,
                    u32 scope,
                    const std::list<Parameter> &parameter_list,
                    Location location);

                const Symbol *insert_variable(
                    const std::string &name,
                    u32 scope,
                    Variable::Space space,
                    u32 offset,
                    Location location);

                const Symbol *lookup_global(const std::string &name) const;
                const Symbol *lookup_chain(const std::string &name, u32 scope) const;
                const Symbol *lookup_local(const std::string &name, u32 scope) const;
                void hide_scope_symbols(u32 scope) noexcept;
                bool is_lib_function(const std::string &name) const;
                const auto &symbols_per_scope() const { return symbols_per_scope_; }

        private:
                SymbolMap symbol_map_;
                // Are inserted in sorted order based on symbol insertion.
                std::vector<std::vector<const Symbol *>> symbols_per_scope_;
                // Are inserted in sorted order based on symbol insertion.
                std::vector<std::vector<Symbol *>> actives_per_scope_;
                std::unordered_set<SymbolName> library_function_set_;

                template <typename SymbolKind, typename... Args>
                        requires std::is_same_v<SymbolKind, Variable> ||
                                 std::is_same_v<SymbolKind, Function>
                const Symbol *insert_symbol(const std::string &name, u32 scope, Args &&...args);
        };
} // namespace Alpha
#endif // SYMBOL_TABLE_HPP
