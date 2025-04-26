#include <string>
#include "core/alpha_shared_interface.hpp" // for ALPHA_YYLEX_SIGNATURE
#include "scanner/alpha_scanner_context.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_error.hpp"
#include "core/alpha_location.hpp"

extern ALPHA_YYLEX_SIGNATURE;
// TODO: say in your report for  the progect that ';' is not just a plain syntax requirement.
// but also the parser's sync point, anything goes wrong, (syntax error) parser can continue parsing gracefully after ';'

#define YYLLOC_DEFAULT(Current, Rhs, N)                                           \
        do                                                                        \
        {                                                                         \
                if ((N))                                                          \
                {                                                                 \
                        (Current).first_index_ = YYRHSLOC((Rhs), 1).first_index_; \
                        (Current).last_index_ = YYRHSLOC((Rhs), N).last_index_;   \
                }                                                                 \
                else                                                              \
                {                                                                 \
                        (Current).first_index_ = YYRHSLOC((Rhs), 0).last_index_;  \
                        (Current).last_index_ = YYRHSLOC((Rhs), 0).last_index_;   \
                }                                                                 \
        } while (0)

static void alpha_yyerror([[maybe_unused]] Alpha::LexerCtx &lexer_ctx,
                          [[maybe_unused]] Alpha::ParseCtx &parse_ctx,
                          [[maybe_unused]] Alpha::SymbolTable &symbol_table,
                          Alpha::ErrorTracker &error_tracker,
                          [[maybe_unused]] Alpha::LocationTracker &lt,
                          std::string error_message)
{
        static constexpr char k_prefix[] = "syntax error, ";
        if (error_message.rfind(k_prefix, 0) == 0)        // does it start with that?
                error_message.erase(0, strlen(k_prefix)); // remove it
        extern Location alpha_yylloc;
        error_tracker.report_error(Alpha::CTError::Type::SYNTAX,
                                   error_message, Location(alpha_yylloc.first_index_, alpha_yylloc.last_index_));
}
