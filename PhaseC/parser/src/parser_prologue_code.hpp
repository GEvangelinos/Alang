#ifndef ALPHA_PARSER_PROLOGUE_CODE_HPP
#define ALPHA_PARSER_PROLOGUE_CODE_HPP

// TODO: use the static SourceLocation function to do the merge.. (both merges) (make a second static func if necessary).
#define YYLLOC_DEFAULT(Current, Rhs, N)                             \
    do                                                              \
    {                                                               \
        if ((N))                                                    \
        {                                                           \
            (Current).first_index = YYRHSLOC((Rhs), 1).first_index; \
            (Current).last_index = YYRHSLOC((Rhs), N).last_index;   \
        }                                                           \
        else                                                        \
        {                                                           \
            (Current).first_index = YYRHSLOC((Rhs), 0).last_index;  \
            (Current).last_index = YYRHSLOC((Rhs), 0).last_index;   \
        }                                                           \
    } while (0)
#include "utils/debug_tools.hpp"

static void alpha_yyerror(
    [[maybe_unused]] ALPHA_YYLTYPE *yylloc,
    [[maybe_unused]] yyscan_t yyscanner,
    [[maybe_unused]] alpha::LexerCtx &lexer_ctx,
    [[maybe_unused]] alpha::LocationTracker &lt,
    [[maybe_unused]] alpha::DiagnosticReporter &dr,
    [[maybe_unused]] alpha::SemanticSystem &ss,
    [[maybe_unused]] std::string error_message)
{
    DEBUG_SMART_ASSERT(false && "alpha_yyerror function called why? Is it too many expected args?");
    // TODO: implement cause even if youuse yyreport_error or what ever it was called to get the unexpected and expected tokens...
    // some intervals of bison still need yyerror to output internal errors, like memory exhaustion , etc.
    //     static constexpr char k_bison_error_prefix[] = "syntax error, ";
    //     if (error_message.rfind(k_bison_error_prefix, 0) == 0)    // does it start with that?
    //         error_message.erase(0, strlen(k_bison_error_prefix)); // remove it
    //     dr.report_parse_error(error_message, alpha_yylloc);
}

#endif // ALPHA_PARSER_PROLOGUE_CODE_HPP
