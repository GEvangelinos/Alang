#ifndef ALPHA_PARSER_PROLOGUE_CODE_HPP
#define ALPHA_PARSER_PROLOGUE_CODE_HPP

#include <string>

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

namespace alpha
{
class LexerCtx;
class LocationTracker;
class DiagnosticEngine;
class DiagnosticReporter;
class SemanticSystem;
} // namespace alpha

static void alpha_yyerror(
    const ALPHA_YYLTYPE *,
    yyscan_t,
    const alpha::LexerCtx &,
    const alpha::LocationTracker &,
    alpha::DiagnosticEngine &,
    const alpha::DiagnosticReporter &,
    const alpha::SemanticSystem &,
    const std::string &error_message);
#endif // ALPHA_PARSER_PROLOGUE_CODE_HPP
