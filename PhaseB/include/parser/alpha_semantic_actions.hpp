#ifndef SEMANTIC_ACTIONS_HPP
#define SEMANTIC_ACTIONS_HPP

#include "core/alpha_error_tracker.hpp"
#include "core/alpha_location.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_symbol_table.hpp"

using namespace Alpha;

void loopCtrlStmt__break(
    const ParseCtx &parse_ctx,
    CodeLocation break_location,
    ErrorTracker &error_tracker);

void loopCtrlStmt__continue(
    const ParseCtx &parse_ctx,
    CodeLocation continue_location,
    ErrorTracker &error_tracker);

void funcDef__function_id_lparen(
    ParseCtx &parse_ctx);

void funcDef__function_id_lparen_idList_rparen(
    SymbolTable &symbol_table,
    ParseCtx &parse_ctx,
    const char *id_name,
    CodeLocation id_location,
    ErrorTracker &error_tracker);

void funcDef__function_lparen(
    ParseCtx &parse_ctx);

void funcDef__function_lparen_idList_rparen(
    SymbolTable &symbol_table,
    ParseCtx &parse_ctx,
    CodeLocation function_location);

void funcArgs__id(
    ParseCtx &parse_ctx,
    const char *id_name,
    CodeLocation id_location);

void funcCtrlStmt__return(
    const ParseCtx &parse_ctx,
    CodeLocation return_location,
    ErrorTracker &error_tracker);

void whileStmt__whileHeader(
    ParseCtx &parse_ctx) noexcept;

void whileStmt__whileHeader_stmt(
    ParseCtx &parse_ctx) noexcept;

void forStmt__forHeader(
    ParseCtx &parse_ctx) noexcept;

void forStmt__forHeader_stmt(
    ParseCtx &parse_ctx) noexcept;

#endif /* SEMANTIC_ACTIONS_HPP */