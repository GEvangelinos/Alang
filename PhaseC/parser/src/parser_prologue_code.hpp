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

static void alpha_yyerror(
    ALPHA_YYLTYPE *,
    yyscan_t,
    alpha::LexerCtx &,
    alpha::LocationTracker &,
    alpha::DiagnosticEngine &,
    alpha::DiagnosticReporter &,
    alpha::SemanticSystem &,
    std::string error_message);
#endif // ALPHA_PARSER_PROLOGUE_CODE_HPP
