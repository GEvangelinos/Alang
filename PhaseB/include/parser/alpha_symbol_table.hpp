// NOTE: Formatting may appear off due to auto-formatter differences between environments.
#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include <optional>
#include "core/alpha_location.hpp"
#include "core/alpha_types.hpp"
#include "core/alpha_error_tracker.hpp"
#include "parser/alpha_parser_context.hpp"

namespace Alpha
{
        class Symbol;
        using SymbolTableEntry = Symbol;

        enum class ScopeType // Todo: Please rename these enums...
        {
                GLOBAL_SCOPE,
                FORMAL_SCOPE,
                LOCAL_SCOPE
        };

        enum class SymbolType
        {
                GLOBAL, // Global variables (scope level 0).
                FORMAL, // Function arguments (formal parameters)
                LOCAL,  // Local variables (non-formal, scope level >= 1)
                USERFUNC,
                LIBFUNC
        };

        struct Parameter
        {
                const std::string &name_;
                const CodeLocation location_;
        };

        static bool is_type_function(SymbolType type)
        {
                return type == SymbolType::LIBFUNC ||
                       type == SymbolType::USERFUNC;
        }

        static bool is_type_varible(SymbolType type) { return !is_type_function(type); }

        class Symbol // Lean version (it doesn't contain name, Symbol Table keeps that as its key).
        {
        public:
                // clang-format off
                SymbolType type()         const noexcept { return type_; }
                u32 scope()               const noexcept { return scope_; }
                CodeLocation location()   const noexcept { return location_; }
                bool is_active()          const noexcept { return is_active_; }
                void activate()                 noexcept { is_active_ = true; }
                void deactivate()               noexcept { is_active_ = false; }
                bool is_function()        const noexcept { return is_type_function(type_); }
                bool is_variable()        const noexcept { return is_type_varible(type_); }
                // clang-format on

        protected:
                Symbol(u32 scope, SymbolType type, CodeLocation location) noexcept
                    : scope_(scope), type_(type), location_(location) {}

        private:
                const u32 scope_;
                const SymbolType type_;
                const CodeLocation location_;
                bool is_active_;
        };

        class SymbolTable
        {
        public:
                void insert_variable(const std::string &name, SymbolType type,
                                     u32 scope, CodeLocation location);

                void insert_function(const std::string &name, SymbolType type,
                                     u32 scope, CodeLocation location,
                                     std::list<Parameter> &&argument_list);

                void insert_anonymous(u32 scope, CodeLocation location,
                                      const std::list<Parameter> &argument_list);

                const Symbol *lookup_global(const std::string &name) const;
                const Symbol *lookup_between(const std::string &name, u32 scope) const;
                const Symbol *lookup_local(const std::string &name, u32 scope) const;
                const Symbol *lookup_function(const std::string &name) const;
                const Symbol *lookup_variable(const std::string &name) const;
                const Symbol *lookup_symbol(const std::string &name) const;

                bool is_lib_function(const std::string &name);

                void hide_current_scope_symbols();

                SymbolTable();
                ~SymbolTable();

        private:
                class Variable;
                class Function;

                template <typename SymbolKind, typename... ArgumentList>
                void insert_symbol(const std::string &name, SymbolType type,
                                   u32 scope, CodeLocation location, ArgumentList &&...arg_list);

                const Symbol *lookup_symbol();

                using SymbolName = std::string;
                using SymbolList = std::list<Symbol>;
                std::unordered_map<SymbolName, SymbolList> symbol_map_;
        };

        class SymbolTable::Variable : public Symbol
        {
        public:
                Variable(u32 scope, SymbolType type, CodeLocation location)
                    : Symbol(scope, type, location) {}
        };

        class SymbolTable::Function : public Symbol
        {
        public:
                Function(u32 scope, SymbolType type, CodeLocation location,
                         std::list<Parameter> &&parameter_list)
                    : Symbol(scope, type, location), parameter_list_(std::move(parameter_list)) {}

        private:
                // TODO: Const list reference or std::move() efficiently?
                std::list<Parameter> parameter_list_;
        };
} /* namespace Alpha */

#endif // SYMBOL_TABLE_HPP
