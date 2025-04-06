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
        static constexpr u32 GLOBAL_SCOPE_DEPTH = 0;
        static const char *ANONYMOUS_FUNCTION_NAME_PREFIX = "SavvidisLemeKaiKleme#";

        enum class ScopeType // Todo: Please rename these enums...
        {
                GLOBAL_SCOPE,
                FORMAL_SCOPE,
                LOCAL_SCOPE
        };

        struct Parameter
        {
                const std::string &name_;
                const CodeLocation location_;
        };

        class Symbol
        {
        public:
                enum class Type
                {
                        GLOBAL, // Global variables (scope level 0).
                        FORMAL, // Function arguments (formal parameters)
                        LOCAL,  // Local variables (non-formal, scope level >= 1)
                        USERFUNC,
                        LIBFUNC
                };
                // clang-format off
                const std::string &name() const noexcept { return name_; }
                Type type()               const noexcept { return type_; }
                u32 scope()               const noexcept { return scope_; }
                CodeLocation location()   const noexcept { return location_; }
                bool is_active()          const noexcept { return is_active_; }
                void activate()                 noexcept { is_active_ = true; }
                void deactivate()               noexcept { is_active_ = false; }
                bool is_function()        const noexcept { return type_ == Type::LIBFUNC ||
                                                                  type_ == Type::USERFUNC; }
                bool is_variable()        const noexcept { return !this->is_function(); }
                // clang-format on

        protected:
                Symbol(const std::string &name, u32 scope, Type type, CodeLocation location) noexcept
                    : name_(name), scope_(scope), type_(type), location_(location) {}

        private:
                const std::string name_;
                const u32 scope_;
                const Type type_;
                const CodeLocation location_;
                bool is_active_;
        };

        class SymbolTable
        {
        public:
                Status insert_variable(const std::string &name, Symbol::Type type, CodeLocation location);
                Status insert_formal_variable(const std::string &name, CodeLocation location);

                Status insert_function(const std::string &name, Symbol::Type type, const std::list<Parameter> &,
                                       CodeLocation location);
                Status insert_anonymous_function(const std::list<Parameter> &, CodeLocation location);

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
                const Symbol *lookup_symbol();
                class Variable;
                class Function;
        };


        class SymbolTable::Variable : public Symbol
        {
        private:
                Variable(const std::string &name, u32 scope, Symbol::Type type, CodeLocation location)
                    : Symbol(name, scope, type, location) {}
        };

        class SymbolTable::Function : public Symbol
        {
        private:
                Function(const std::string &name, u32 scope, Symbol::Type type,
                         std::list<Parameter> &&parameter_list, CodeLocation location)
                    : Symbol(name, scope, type, location), parameter_list_(std::move(parameter_list)) {}

                    // TODO: Const list reference or std::move() efficiently?
                std::list<Parameter> parameter_list_;
        };
} /* namespace Alpha */

#endif // SYMBOL_TABLE_HPP
