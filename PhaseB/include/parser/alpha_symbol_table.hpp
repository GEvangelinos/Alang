// NOTE: Formatting may appear off due to auto-formatter differences between environments.
#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <cstdint>
#include <list>
#include <string>
#include <set>
#include <unordered_map>
#include <vector>
#include <stack>
#include <optional>
#include "core/alpha_location.hpp"
#include "core/alpha_types.hpp"
#include "core/alpha_error_tracker.hpp"
#include "parser/alpha_parser_context.hpp"
#include "_parser_common.hpp"
#include <unordered_set>
#include <initializer_list>

namespace Alpha
{

        class Symbol;

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

        enum class SymbolType
        {
                GLOBAL, // Global variables (scope level 0).
                FORMAL, // Function arguments (formal parameters)
                LOCAL,  // Local variables (non-formal, scope level >= 1)
                USERFUNC,
                LIBFUNC
        };

        inline std::string_view to_string(SymbolType type)
        {
                switch (type)
                {
                case Alpha::SymbolType::LIBFUNC:
                        return "LIBRARY_FUNCTION";
                case Alpha::SymbolType::GLOBAL:
                        return "GLOBAL_VARIABLE";
                case Alpha::SymbolType::USERFUNC:
                        return "USER_FUNCTION";
                case Alpha::SymbolType::FORMAL:
                        return "FORMAL_ARGUMENT";
                case Alpha::SymbolType::LOCAL:
                        return "LOCAL_VARIABLE";
                }
                UNREACHABLE("You forgot to register a `SymbolType`");
        }

        static DEBUG_ALWAYS_INLINE bool is_type_function(SymbolType type)
        {
                return type == SymbolType::LIBFUNC ||
                       type == SymbolType::USERFUNC;
        }

        class SymbolTable
        {
        public:
                using SymbolName = std::string;
                using SymbolMap = std::unordered_map<SymbolName, std::list<Symbol>>;
                using ScopeID = u32;

                SymbolTable();
                ~SymbolTable() = default;
                SymbolTable(const SymbolTable &) = delete;
                SymbolTable(const SymbolTable &&) = delete;
                SymbolTable &operator=(const SymbolTable &) = delete;
                SymbolTable &operator=(const SymbolTable &&) = delete;

                const Symbol *insert_global(const std::string &symbol_name, Location location);
                const Symbol *insert_formal(const std::string &symbol_name, u32 scope,
                                            Location location);
                const Symbol *insert_local(const std::string &symbol_name, u32 scope,
                                           Location location);

                const Symbol *insert_function(const std::string &symbol_name, SymbolType type,
                                              u32 scope, Location location,
                                              const std::list<Parameter> &argument_list);
                const Symbol *insert_anonymous(u32 scope, Location location,
                                               const std::list<Parameter> &argument_list);

                const Symbol *lookup_global(const std::string &symbol_name) const;
                const Symbol *lookup_chain(const std::string &symbol_name, u32 scope) const;
                const Symbol *lookup_local(const std::string &symbol_name, u32 scope) const;

                bool is_lib_function(const std::string &symbol_name) const;

                void hide_scope_symbols(u32 scope) noexcept;

                const auto &symbols_per_scope() const { return symbols_per_scope_; }

        private:
                class Variable;
                class Function;

                template <typename SymbolKind, typename... ParameterList>
                        requires(std::is_same_v<SymbolKind, Variable> ||
                                 std::is_same_v<SymbolKind, Function>)
                const Symbol *insert_symbol(const std::string &symbol_name, SymbolType type,
                                            u32 scope, Location location, ParameterList &&...arg_list);

                u32 anonymous_counter_;
                SymbolMap symbol_map_;
                std::vector<std::vector<const Symbol *>> symbols_per_scope_; // Are inserted in sorted order based on symbol insertion.
                std::vector<std::vector<Symbol *>> actives_per_scope_; // Are inserted in sorted order based on symbol insertion.

                std::unordered_set<SymbolName> library_function_set_;
        };

        static DEBUG_ALWAYS_INLINE bool is_type_varible(SymbolType type) { return !is_type_function(type); }

        class Symbol // Lean version (it doesn't contain name, Symbol Table keeps that as its key).
        {
        public:
                // clang-format off
                DEBUG_ALWAYS_INLINE SymbolType type()   const noexcept { return type_; }
                DEBUG_ALWAYS_INLINE u32 scope()         const noexcept { return scope_; }
                DEBUG_ALWAYS_INLINE Location location() const noexcept { return location_; }
                DEBUG_ALWAYS_INLINE bool is_function()  const noexcept { return is_type_function(type_); }
                DEBUG_ALWAYS_INLINE bool is_variable()  const noexcept { return is_type_varible(type_); }
                DEBUG_ALWAYS_INLINE const std::string& name() const noexcept { return name_;}
                DEBUG_ALWAYS_INLINE bool is_active() const noexcept { return is_active_; }
                // clang-format on

        protected:
                Symbol(const std::string &name, u32 scope, SymbolType type, Location location) noexcept
                    : name_(name),
                      scope_(scope),
                      type_(type),
                      location_(location) { activate(); }

        private:
                const std::string &name_;
                const u32 scope_;
                const SymbolType type_;
                const Location location_;
                bool is_active_;

                DEBUG_ALWAYS_INLINE void activate() noexcept { is_active_ = true; }
                DEBUG_ALWAYS_INLINE void deactivate() noexcept { is_active_ = false; }

                friend class SymbolTable;
        };

        class SymbolTable::Variable : public Symbol
        {
        public:
                Variable(const std::string &name, u32 scope, SymbolType type, Location location)
                    : Symbol(name, scope, type, location) {}
        };

        class SymbolTable::Function : public Symbol
        {
        public:
                Function(const std::string &name, u32 scope, SymbolType type, Location location,
                         const std::list<Parameter> &parameter_list)
                    : Symbol(name, scope, type, location), parameter_list_(parameter_list) {}

        private:
                const std::list<Parameter> parameter_list_;
        };
} /* namespace Alpha */

#endif // SYMBOL_TABLE_HPP
