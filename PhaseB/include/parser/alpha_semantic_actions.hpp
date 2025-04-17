#ifndef SEMANTIC_ACTIONS_HPP
#define SEMANTIC_ACTIONS_HPP

#include "core/alpha_error_tracker.hpp"
#include "core/alpha_location.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_symbol_table.hpp"

using namespace Alpha;

void loopCtrlStmt__break(
    const ParseCtx &parse_ctx,
    Location break_location,
    ErrorTracker &et);

void loopCtrlStmt__continue(
    const ParseCtx &parse_ctx,
    Location continue_location,
    ErrorTracker &et);

void lvalue__id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_location,
    const Symbol **lvalue,
    ErrorTracker &et);

void lvalue__local_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_location,
    const Symbol **lvalue,
    ErrorTracker &et);

void lvalue__global_id(
    SymbolTable &st,
    const char *id_name,
    Location id_location,
    const Symbol **lvalue,
    ErrorTracker &et);

void funcDef__function_id_lparen_idList_rparen(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_location,
    ErrorTracker &et);

void funcDef__function_lparen_idList_rparen(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    Location function_location,
    ErrorTracker &et);

void funcDef__function_id_lparen_idList_rparen_block(
    ParseCtx &parse_ctx);

void funcDef__function_lparen_idList_rparen_block(
    ParseCtx &parse_ctx);

void funcArgs__id(
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_location);

void funcCtrlStmt__return(
    const ParseCtx &parse_ctx,
    Location return_location,
    ErrorTracker &et);

void block__lbrace(
    ParseCtx &parse_ctx);

void block__lbrace_multiStmt_rbrace(
    SymbolTable &st,
    ParseCtx &parse_ctx);

void block__lbrace_rbrace(
    SymbolTable &st,
    ParseCtx &parse_ctx);

void whileStmt__whileHeader(
    ParseCtx &parse_ctx) noexcept;

void whileStmt__whileHeader_stmt(
    ParseCtx &parse_ctx) noexcept;

void forStmt__forHeader(
    ParseCtx &parse_ctx) noexcept;

void forStmt__forHeader_stmt(
    ParseCtx &parse_ctx) noexcept;

#endif /* SEMANTIC_ACTIONS_HPP */