#ifndef SEMANTIC_ACTIONS_HPP
#define SEMANTIC_ACTIONS_HPP

#include <string>                          // for string
#include "core/alpha_error.hpp"            // for ErrorTracker
#include "core/alpha_location.hpp"         // for Location
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "parser/alpha_symbol_table.hpp"   // for Symbol, SymbolTable

using namespace Alpha;

void loopCtrlStmt__break(const ParseCtx &parse_ctx, Location break_location, ErrorTracker &et);

void loopCtrlStmt__continue(const ParseCtx &parse_ctx, Location continue_location, ErrorTracker &et);

void term__inc_lvalue(const Symbol *lvalue, Location term_location, ErrorTracker &et);

void term__lvalue_inc(const Symbol *lvalue, Location term_location, ErrorTracker &et);

void term__dec_lvalue(const Symbol *lvalue, Location term_location, ErrorTracker &et);

void term__lvalue_dec(const Symbol *lvalue, Location term_location, ErrorTracker &et);

void assignExpr__lvalue_assign_expr(
    const Symbol *lvalue,
    Location assign_location,
    ErrorTracker &et);

void lvalue__id(
    SymbolTable &st,
    const ParseCtx &parse_ctx,
    const std::string &id_name,
    Location id_location,
    const Symbol *&lvalue,
    ErrorTracker &et);

void lvalue__local_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const std::string &id_name,
    Location id_location,
    const Symbol *&lvalue,
    ErrorTracker &et);

void lvalue__global_id(
    SymbolTable &st,
    const std::string &id_name,
    Location id_location,
    const Symbol *&lvalue,
    ErrorTracker &et);

void lvalue__member(const Symbol *&lvalue) noexcept;

void blockOpen__lbrace(ParseCtx &parse_ctx) noexcept;

void blockClose__rbrace(SymbolTable &st, ParseCtx &parse_ctx) noexcept;

void funcPrefix__function(ParseCtx &parse_ctx, Location anonymous_location);

void funcPrefix__function_id(ParseCtx &parse_ctx, const std::string &id_name, Location id_location);

void funcSignature__funcPrefix_funcArgList(SymbolTable &st, ParseCtx &parse_ctx, ErrorTracker &et);

void funcDef__funcSignature_block(ParseCtx &parse_ctx) noexcept;

void const__stringliteral(char *&string_literal_addr);

void funcArgs__id(ParseCtx &parse_ctx, const std::string &id_name, Location id_location);

void whileStmt__whileHeader(ParseCtx &parse_ctx) noexcept;

void whileStmt__whileHeader_stmt(ParseCtx &parse_ctx) noexcept;

void forStmt__forHeader(ParseCtx &parse_ctx) noexcept;

void forStmt__forHeader_stmt(ParseCtx &parse_ctx) noexcept;

void funcCtrlStmt__return(const ParseCtx &parse_ctx, Location return_location, ErrorTracker &et);

#endif /* SEMANTIC_ACTIONS_HPP */