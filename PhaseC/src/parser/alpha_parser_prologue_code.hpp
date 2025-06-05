#ifndef ALPHA_PARSER_PROLOGUE_CODE_HPP
#define ALPHA_PARSER_PROLOGUE_CODE_HPP

#include <string>

#include "core/alpha_diagnostics.hpp"
#include "core/alpha_shared_interface.hpp" // for ALPHA_YYLEX_SIGNATURE
#include "scanner/alpha_scanner_context.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_location.hpp"

#include "parser/alpha_semantic_driver.hpp"

extern ALPHA_YYLEX_SIGNATURE;
// TODO: say in your report for  the progect that ';' is not just a plain syntax requirement.
// but also the parser's sync point, anything goes wrong, (syntax error) parser can continue parsing gracefully after ';'

#define YYLLOC_DEFAULT(Current, Rhs, N)                                         \
        do                                                                      \
        {                                                                       \
                if ((N))                                                        \
                {                                                               \
                        (Current).first_index = YYRHSLOC((Rhs), 1).first_index; \
                        (Current).last_index = YYRHSLOC((Rhs), N).last_index;   \
                }                                                               \
                else                                                            \
                {                                                               \
                        (Current).first_index = YYRHSLOC((Rhs), 0).last_index;  \
                        (Current).last_index = YYRHSLOC((Rhs), 0).last_index;   \
                }                                                               \
        } while (0)

static void alpha_yyerror(
    [[maybe_unused]] Alpha::LocationTracker &lt,
    Alpha::Diagnostics &diagnostics,
    [[maybe_unused]] Alpha::LexerCtx &lexer_ctx,
    [[maybe_unused]] Alpha::SemanticDriver &sd,
    std::string error_message)
{
    static constexpr char k_prefix[] = "syntax error, ";
    if (error_message.rfind(k_prefix, 0) == 0)        // does it start with that?
        error_message.erase(0, strlen(k_prefix)); // remove it
    extern Alpha::SourceLocation alpha_yylloc;
    diagnostics.report(
        Alpha::Issue::Type::ERROR,
        error_message,
        Alpha::SourceLocation{
            .first_index = alpha_yylloc.first_index,
            .last_index = alpha_yylloc.last_index
        }
    );
}

#endif // ALPHA_PARSER_PROLOGUE_CODE_HPP
