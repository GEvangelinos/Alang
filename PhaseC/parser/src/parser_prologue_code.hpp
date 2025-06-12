#ifndef ALPHA_PARSER_PROLOGUE_CODE_HPP
#define ALPHA_PARSER_PROLOGUE_CODE_HPP

#include <string>

#include "diagnostics/diagnostic_engine.hpp"
#include "core/shared_interface.hpp" // for ALPHA_YYLEX_SIGNATURE
#include "scanner/scanner_context.hpp"
#include "parser/parser_context.hpp"
#include "parser/symbol_table.hpp"
#include "core/source_location.hpp"

#include "L1_driver/semantic_driver.hpp"

extern ALPHA_YYLEX_SIGNATURE;
// TODO: say in your report for  the project that ';' is not just a plain syntax requirement.
// but also the parser's sync point, anything goes wrong, (syntax error) parser can continue parsing gracefully after ';'


// TODO: use the static SourceLocation function to do the merge.. (both merges) (make a second static func if necessary).
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
    Alpha::DiagnosticEngine &diagnostic_engine,
    [[maybe_unused]] Alpha::LexerCtx &lexer_ctx,
    [[maybe_unused]] Alpha::SemanticDriver &sd,
    std::string error_message)
{
    static constexpr char k_prefix[] = "syntax error, ";
    if (error_message.rfind(k_prefix, 0) == 0) // does it start with that?
        error_message.erase(0, strlen(k_prefix)); // remove it
    extern Alpha::SourceLocation alpha_yylloc;
    // TODO: uncomment and adjust it to new DiagnosticEngine.
    // diagnostic_engine.report(
    //     Alpha::Issue::Type::ERROR,
    //     error_message,
    //     Alpha::SourceLocation{
    //         .first_index = alpha_yylloc.first_index,
    //         .last_index = alpha_yylloc.last_index
    //     }
    // );
}

#endif // ALPHA_PARSER_PROLOGUE_CODE_HPP
