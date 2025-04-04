#ifndef SEMANTIC_ACTIONS_HPP
#define SEMANTIC_ACTIONS_HPP

#include "core/alpha_error_tracker.hpp"
#include "core/alpha_location.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_symbol_table.hpp"

namespace Alpha
{

    void stmt__break_semicolon(
        const ParserContext &parser_context,
        ErrorTracker &error_tracker,
        const Location &break_location);

    void stmt__continue_semicolon(
        const ParserContext &parser_context,
        ErrorTracker &error_tracker,
        const Location &continue_location);

    void term__plusplus_lvalue(
        ErrorTracker &error_tracker,
        const SymbolTableEntry *lvalue,
        const Location &term_location);

    void term__lvalue_plusplus(
        ErrorTracker &error_tracker,
        const SymbolTableEntry *lvalue,
        const Location &term_location);

    void term__minusminus_lvalue(
        ErrorTracker &error_tracker,
        const SymbolTableEntry *const lvalue,
        const Location &term_location);

    void term__lvalue_minusminus(
        ErrorTracker &error_tracker,
        const SymbolTableEntry *lvalue,
        const Location &term_location);

    void assignexpr__lvalue(
        ParserContext &parser_context,
        ErrorTracker &error_tracker,
        const SymbolTableEntry *lvalue,
        const Location &lvalue_location);

    void lvalue__id(
        SymbolTable &symbol_table,
        ErrorTracker &error_tracker,
        const SymbolTableEntry **lvalue,
        const char *id_name,
        const Location &id_location);

    void lvalue__local_id(
        SymbolTable &symbol_table,
        ErrorTracker &error_tracker,
        const SymbolTableEntry **lvalue,
        const char *id_name,
        const Location &lvalue_location,
        const Location &id_location);

    void lvalue__colonblock_id(
        const SymbolTable &symbol_table,
        ErrorTracker &error_tracker,
        const SymbolTableEntry **lvalue,
        const char *id_name,
        const Location &lvalue_location);

    void lvalue__member(ParserContext &parser_context);

    void block__leftbrace(SymbolTable &symbol_table);

    void block__leftbrace_multistmt_rightbrace(SymbolTable &symbol_table);

    void block__leftbrace_rightbrace(SymbolTable &symbol_table);

    void funcdef__function_id_lparen_idlist_rparen(
        SymbolTable &symbol_table,
        ParserContext &parser_context,
        ErrorTracker &error_tracker,
        const char *const id_name,
        const Location &funcdef_location);

    void funcdef__function_id_lparen_idlist_rparen_block(ParserContext &parser_context);

    void funcdef__function_lparen_idlist_rparen(
        SymbolTable &symbol_table,
        ParserContext &parser_context,
        const Location &funcdef_location);

    void funcdef__function_lparen_idlist_rparen_block(ParserContext &parser_context);

    void csids__id(ParserContext &parser_context, const char *const id_name);

    void whilestmt__while_lparen_expr_rparen(ParserContext &parser_context);

    void whilestmt__while_lparen_expr_rparen_stmt(ParserContext &parser_context);

    void forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen(ParserContext &parser_context);

    void forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen_stmt(ParserContext &parser_context);

    void returnstmt__return_semicolon(
        const ParserContext &parser_context,
        ErrorTracker &error_tracker,
        const Location &return_location);

    void returnstmt__return_expr_semicolon(
        const ParserContext &parser_context,
        ErrorTracker &error_tracker,
        const Location &return_location);
} /* namespace Alpha */

#endif /* SEMANTIC_ACTIONS_HPP */