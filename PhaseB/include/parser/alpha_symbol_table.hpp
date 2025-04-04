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
        static constexpr SourceRange LIBFUNC_SOURCE_RANGE = SourceRange(0, 0);
        static const std::list<std::pair<std::string, Location>> LIBFUNC_ARGUMENT_INFO;
        static constexpr std::string ANONYMOUS_FUNCTION_NAME_PREFIX = "#f_";

        enum class SymbolType
        {
                GLOBAL, /* Global variables (scope level 0). */
                FORMAL, /* Function arguments (formal parameters) */
                LOCAL,  /* Local variables (non-formal, scope level >= 1) */
                USERFUNC,
                LIBFUNC
        };

        enum class ScopeType // Todo: Please rename these enums...
        {
                GLOBAL_SCOPE,
                FORMAL_SCOPE,
                LOCAL_SCOPE
        };

        class SymbolTableEntry
        {
        public:
                const std::string &name() const noexcept;
                u32 scope() const noexcept;
                SourceRange source_range() const noexcept;
                u32 line() const; /* TODO: Calculate it useing ranges.*/
                SymbolType type() const noexcept;
                bool is_active() const noexcept;
                void activate() noexcept;
                void deactivate() noexcept;

                SymbolTableEntry() = delete;

        protected:
                SymbolTableEntry(const std::string &name, u32 scope, SourceRange source_range, SymbolType type) noexcept;

        private:
                const std::string name_;
                const u32 scope_;
                const SourceRange source_range_;
                const SymbolType type_;
                bool is_active_;
        };
        class SymbolTable
        {
        public:
                const SymbolTableEntry *insert_variable(
                    const std::string &name,
                    std::optional<SymbolType> symbol_type_opt,
                    const SourceRange &source_range);

                const SymbolTableEntry *insert_function(
                    std::optional<std::string> name,
                    SymbolType symbol_type,
                    const std::list<std::pair<std::string, Location>> &arguments_info,
                    const SourceRange &source_range);

                const SymbolTableEntry *lookup_global(const std::string &name) const;
                const SymbolTableEntry *lookup_local(const std::string &name) const;
                const SymbolTableEntry *lookup_chain(const std::string &name, const SymbolType type) const; // Inclusive to current and global scope.
                const SymbolTableEntry *lookup_function(const std::string &name) const;                     // Inclusive to current and global scope.
                const SymbolTableEntry *lookup_variable(const std::string &name) const;                     // Inclusive to current and global scope.
                const SymbolTableEntry *lookup_symbol(const std::string &name) const;                       // Inclusive to current and global scope.

                bool is_library_function(const std::string &name);
                bool is_in_function_scope();
                void hide_current_scope_symbols();

                u32 active_scope_depth() const;
                void enter_scope(bool is_function_scope);
                void exit_scope();

                void push_new_loop_depth_counter();
                void pop_loop_depth_counter();
                u32 active_loop_depth() const;
                void enter_loop();
                void exit_loop();

                void print_symbol_insertion_vector();

                SymbolTable(ParserContext &parser_context, ErrorTracker &error_tracker_ref);
                ~SymbolTable();

        private:
                u32 active_scope_depth_;
                u32 line_of_last_function;
                u32 maximum_reached_scope_depth_;
                u32 anonymous_function_counter_;
                u32 total_symbol_count_;
                std::unordered_map<std::string, std::list<SymbolTableEntry *>> symbol_map_;
                std::unordered_map<u32, std::vector<SymbolTableEntry *>> symbol_insertion_map_;
                std::vector<ScopeType> scope_type_vector_; /* Add frames with insert, remove with hide. */
                std::stack<u32> loop_depth_counter_stack_; /* 1 Stack-frame per function. */

                ErrorTracker &error_tracker_;   /* COUPLED... :(O) */
                ParserContext &parser_context_; /* COUPLED... :(O) */

                // TODO: Implement a stack of vectors of strings (or references/pointers) to hide effieciently symvols.
                const SymbolTableEntry *insert_entry(SymbolTableEntry *entry_ptr);
                const SymbolTableEntry *lookup_at_scope_depth(const std::string &name, u32 scope_depth) const;
        };

        class VariableEntry : public SymbolTableEntry
        {
        private:
                VariableEntry(const std::string &name, u32 scope, SourceRange source_range, SymbolType type);

                friend const SymbolTableEntry *SymbolTable::insert_variable(
                    const std::string &name,
                    std::optional<SymbolType> symbol_type_opt,
                    const SourceRange &source_range);
        };

        class FunctionEntry : public SymbolTableEntry
        {
        private:
                FunctionEntry(
                    const std::string &name,
                    u32 scope,
                    SourceRange source_range,
                    SymbolType type,
                    const std::list<std::string> &argument_names);

                std::list<std::string> argument_name_list_;

                friend const SymbolTableEntry *SymbolTable::insert_function(
                    std::optional<std::string>,
                    SymbolType,
                    const std::list<std::pair<std::string, Location>> &,
                    const SourceRange &);
        };

} /* namespace Alpha */

#endif /* SYMBOL_TABLE_HPP */
