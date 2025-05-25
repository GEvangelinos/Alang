#ifndef ALPHA_SYMBOLS_HPP
#define ALPHA_SYMBOLS_HPP

#include "core/alpha_location.hpp"
#include <string>
#include "_parser_common.hpp"
#include <list>

namespace Alpha
{
        // Classes defined here:
        class Symbol;   // IWYU pragma: keep
        class Variable; // IYU pragma: keep
        class Function; // IWYU pragma: keep

        class Symbol // Lean version (it doesn't contain name, Symbol Table keeps that as its key).
        {
        public:
                enum class Type : u8
                {
                        LIBRARY_FUNCTION,
                        PROGRAM_FUNCTION,
                        FORMAL_ARGUMENT,
                        GLOBAL_VARIABLE,
                        LOCAL_VARIABLE,
                };

                const std::string &name;
                const u32 scope;
                const Type type;
                const SourceLocation location;

                virtual ~Symbol() = default;

                [[nodiscard]] std::string_view type_to_string() const noexcept;
                // TODO REMOVE // [[nodiscard]] bool is_variable() const noexcept { return type == Type::VARIABLE; }
                [[nodiscard]] bool is_variable() const noexcept { return !is_function(); }
                [[nodiscard]] bool is_function() const noexcept
                {
                        return type == Type::LIBRARY_FUNCTION ||
                               type == Type::PROGRAM_FUNCTION;
                }
                [[nodiscard]] bool is_active() const noexcept { return is_active_; }
                [[nodiscard]] static bool is_modifiable_symbol(const Symbol *symbol);

        protected:
                Symbol(const std::string &name, u32 scope, Type type, SourceLocation loc) noexcept
                    : name(name), scope(scope), type(type), location(loc) {}

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

                Variable(
                    const std::string &name,
                    u32 scope,
                    Type type,
                    Space space,
                    u32 offset,
                    SourceLocation loc)
                    : Symbol(name, scope, type, loc),
                      space(space),
                      offset(offset) {}
                ~Variable() override = default;
        };

        class Function : public Symbol
        {
        public:
                const u32 address;
                const std::list<Parameter> parameter_list; // TODO: change to vector (cache friendly...)
                Once<u32> local_variable_count;

                Function(
                    const std::string &name,
                    const u32 scope,
                    const Symbol::Type type,
                    const u32 address,
                    const std::list<Parameter> &parameter_list,
                    const SourceLocation location)
                    : Symbol(name, scope, type, location),
                      address(address),
                      parameter_list(parameter_list)
                {
                        DEBUG_SMART_ASSERT(
                            type == Symbol::Type::LIBRARY_FUNCTION ||
                            type == Symbol::Type::PROGRAM_FUNCTION //
                        );
                }

                ~Function() override = default;
        };

        inline bool
        Symbol::is_modifiable_symbol(const Symbol *symbol)
        {
                // TODO: remove (deprecated part from phase 2)
                // if (!symbol) // nullptr implies runtime-evaluated lvalue (e.g. member access)
                // 	return true;
                return symbol->is_variable();
        }

} // namespace Alpha
#endif // ALPHA_SYMBOLS_HPP