#ifndef PRELUDEC_DOE_HPP
#define PRELUDEC_DOE_HPP
#include <string>
#include <list>
#include <iostream>
#include <stdexcept>
#include "alpha_scanner.hpp"
#include "parser/alpha_logger.hpp"
#include "parser/alpha_semantic_actions.hpp"
#include "core/alpha_shared_interface.hpp"
#include "scanner/alpha_scanner_context.hpp"
#include "parser/alpha_parser_context.hpp"
static void alpha_yyerror(Alpha::LexerCtx &lexer_ctx,
                          Alpha::ParseCtx &parse_ctx,
                          Alpha::SymbolTable &symbol_table,
                          Alpha::ErrorTracker &error_tracker,
                          Alpha::LocationTracker &lt,
                          const std::string &error_message)
{
    error_tracker.report_syntax_error(error_message, Location(10, 20));
}
#endif // PRELUDEC_DOE_HPP