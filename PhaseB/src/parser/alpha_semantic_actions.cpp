#include <format>
#include "core/alpha_error_tracker.hpp"
#include "misc/smart_assert.h"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_symbol_table.hpp"

#ifdef DEBUG_MODE
/* In debug mode, disable inlining to ensure the function has a visible symbol for debugging. */
#undef __always_inline
#define __always_inline
#endif

namespace /* (Anonymous) */
{
        using namespace Alpha;

        void __always_inline impl_unassignable_expression(
            ErrorTracker &error_tracker,
            const SymbolTableEntry *const lvalue,
            const Location &term_location)
        {
                DEBUG_SMART_ASSERT(lvalue != nullptr);

                if (lvalue->type() != SymbolType::LIBFUNC && lvalue->type() != SymbolType::USERFUNC)
                        return;

                error_tracker.register_compile_time_error(new SyntaxError(term_location, "expression is not assignable"));
        }

        void __always_inline impl_stmt_in_loop_context(
            const SymbolTable &symbol_table,
            ErrorTracker &error_tracker,
            const Location &keyword_location,
            const std::string &keyword_name)
        {
                if (symbol_table.active_loop_depth() > 0)
                        return;

                error_tracker.register_compile_time_error(new SyntaxError(
                    keyword_location, std::format("`{}` statement not within a loop statement", keyword_name)));
        }
}

namespace Alpha
{
        void stmt__break_semicolon(
            const SymbolTable &symbol_table,
            ErrorTracker &error_tracker,
            const Location &break_location)
        {
                impl_stmt_in_loop_context(symbol_table, error_tracker, break_location, "break");
        }

        void stmt__continue_semicolon(
            const SymbolTable &symbol_table,
            ErrorTracker &error_tracker,
            const Location &continue_location)
        {
                impl_stmt_in_loop_context(symbol_table, error_tracker, continue_location, "continue");
        }

        void term__plusplus_lvalue(
            ErrorTracker &error_tracker,
            const SymbolTableEntry *const lvalue,
            const Location &term_location)
        {
                DEBUG_SMART_ASSERT(lvalue != nullptr);
                impl_unassignable_expression(error_tracker, lvalue, term_location);
        }

        void term__lvalue_plusplus(
            ErrorTracker &error_tracker,
            const SymbolTableEntry *const lvalue,
            const Location &term_location)
        {
                DEBUG_SMART_ASSERT(lvalue != nullptr);
                impl_unassignable_expression(error_tracker, lvalue, term_location);
        }

        void term__minusminus_lvalue(
            ErrorTracker &error_tracker,
            const SymbolTableEntry *const lvalue,
            const Location &term_location)
        {
                DEBUG_SMART_ASSERT(lvalue != nullptr);
                impl_unassignable_expression(error_tracker, lvalue, term_location);
        }

        void term__lvalue_minusminus(
            ErrorTracker &error_tracker,
            const SymbolTableEntry *const lvalue,
            const Location &term_location)
        {
                DEBUG_SMART_ASSERT(lvalue != nullptr);
                impl_unassignable_expression(error_tracker, lvalue, term_location);
        }

        void assignexpr__lvalue(
            ParserContext &parser_context,
            ErrorTracker &error_tracker,
            const SymbolTableEntry *lvalue,
            const Location &lvalue_location)
        {
                DEBUG_SMART_ASSERT(lvalue != nullptr);

                /* NOTE(6243636488438628): Well if lvalue is in runtime we should check if that member is a variable or a member function. */
                if (!parser_context.lvalue_is_member())
                        impl_unassignable_expression(error_tracker, lvalue, lvalue_location);

                parser_context.clear_lvalue_is_member();
        }

        void lvalue__id(
            SymbolTable &symbol_table,
            ErrorTracker &error_tracker,
            const SymbolTableEntry **lvalue,
            const char *const id_name,
            const Location &id_location)
        {
                const SymbolTableEntry *entry_ptr = symbol_table.lookup_symbol(id_name);
                if (entry_ptr == nullptr)
                {
                        entry_ptr = symbol_table.insert_variable(id_name,std::nullopt, id_location);
                        DEBUG_SMART_ASSERT(symbol_table.lookup_local(id_name) != nullptr);
                }
                else if (symbol_table.is_symbol_outisde_function(id_name))
                {
                        /* TODO: FIND out what happens if symbol is LIBFUNC, can the path even take us here? */
                        const std::string error_message = std::format(
                            "variable `{}` is declared outside current function scope in line {}", entry_ptr->name(), entry_ptr->line());
                        error_tracker.register_compile_time_error(new SyntaxError(id_location, error_message));
                }
                *lvalue = entry_ptr;
        }

        void lvalue__local_id(
            SymbolTable &symbol_table,
            ErrorTracker &error_tracker,
            const SymbolTableEntry **lvalue,
            const char *const id_name,
            const Location &lvalue_location,
            const Location &id_location)
        {
                const SymbolTableEntry *entry_ptr = symbol_table.lookup_global(id_name);
                if (entry_ptr && (entry_ptr->type() == SymbolType::LIBFUNC))
                {
                        const std::string error_message = std::format("`{}` shadows library function", id_name);
                        error_tracker.register_compile_time_error(new SyntaxError(lvalue_location, error_message));
                }
                else if ((entry_ptr = symbol_table.lookup_local(id_name)) == nullptr) /* if control here, then not a LIBFUNC. */
                {
                        entry_ptr = symbol_table.insert_variable(id_name, id_location);
                        DEBUG_SMART_ASSERT(symbol_table.lookup_local_symbol(id_name) != nullptr);
                }
                *lvalue = entry_ptr;
        }

        void lvalue__colonblock_id(
            const SymbolTable &symbol_table,
            ErrorTracker &error_tracker,
            const SymbolTableEntry **lvalue,
            const char *const id_name,
            const Location &lvalue_location)
        {
                const SymbolTableEntry *entry_ptr = symbol_table.lookup_global(id_name);
                if (entry_ptr == nullptr)
                {
                        const std::string error_message = std::format("::{} not found in global scope", id_name);
                        error_tracker.register_compile_time_error(new SyntaxError(lvalue_location, error_message));
                }
                *lvalue = entry_ptr;
        }

        void lvalue__member(ParserContext &parser_context)
        {
                parser_context.set_lvalue_is_member();
        }

        void block__leftbrace(SymbolTable &symbol_table)
        {
                symbol_table.enter_scope();
        }

        void block__leftbrace_multistmt_rightbrace(SymbolTable &symbol_table)
        {
                symbol_table.exit_scope();
        }

        void block__leftbrace_rightbrace(SymbolTable &symbol_table)
        {
                symbol_table.exit_scope();
        }

        void funcdef__function_id_lparen_idlist_rparen(
            SymbolTable &symbol_table,
            ParserContext &parser_context,
            ErrorTracker &error_tracker,
            const char *const id_name,
            const Location &funcdef_location)
        {
                std::string error_message = "";
                const SymbolTableEntry *local_entry_ptr = symbol_table.lookup_local(id_name);
                if (symbol_table.is_library_function(id_name))
                        error_message = std::format("redefinition of library function `{}`", id_name);
                else if (local_entry_ptr && local_entry_ptr->type() == Alpha::SymbolType::USERFUNC)
                        error_message = std::format("redefintiion of function `{}` in active scope", id_name);
                else if (local_entry_ptr) /* We found a symbol, and it was a neither LIBFUNC nor a USERFUNC, thus it is a variable. */
                        error_message = std::format("`{}` is already defined as a variable", id_name);
                else
                        symbol_table.insert_function(id_name, parser_context.function_argument_list(), SymbolType::USERFUNC, funcdef_location);

                if (!error_message.empty())
                        error_tracker.register_compile_time_error(new SyntaxError(funcdef_location, error_message));
                parser_context.enter_function();
        }

        void funcdef__function_id_lparen_idlist_rparen_block(ParserContext &parser_context)
        {
                parser_context.exit_function();
        }

        void funcdef__function_lparen_idlist_rparen(
            SymbolTable &symbol_table,
            ParserContext &parser_context,
            const Location &funcdef_location)
        {
                symbol_table.insert_function(std::nullopt, parser_context.function_argument_list(), SymbolType::USERFUNC, funcdef_location);
                parser_context.enter_function();
        }

        void funcdef__function_lparen_idlist_rparen_block(ParserContext &parser_context)
        {
                parser_context.exit_function();
        }

        void csids__id(ParserContext &parser_context, const char *const id_name, const Location &id_location)
        {
                parser_context.append_function_argument(id_name, id_location);
        }

        void whilestmt__while_lparen_expr_rparen(ParserContext &parser_context)
        {
                parser_context.enter_loop();
        }

        void whilestmt__while_lparen_expr_rparen_stmt(ParserContext &parser_context)
        {
                parser_context.exit_loop();
        }

        void forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen(ParserContext &parser_context)
        {
                parser_context.enter_loop();
        }

        void forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen_stmt(ParserContext &parser_context)
        {
                parser_context.exit_loop();
        }

        void returnstmt__return_semicolon(
            const ParserContext &parser_context,
            ErrorTracker &error_tracker,
            const Location &return_location)
        {
                if (parser_context.active_function_depth() == 0)
                        error_tracker.register_compile_time_error(new SyntaxError(
                            return_location, "`return` statement not within a function statement"));
        }

        void returnstmt__return_expr_semicolon(
            const ParserContext &parser_context,
            ErrorTracker &error_tracker,
            const Location &return_location)
        {
                returnstmt__return_semicolon(parser_context, error_tracker, return_location);
        }
}