#ifndef ALPHA_PARSER_PROLOGUE_CODE_HPP
#define ALPHA_PARSER_PROLOGUE_CODE_HPP

#include <string>
// TODO: use the static SourceLocation function to do the merge.. (both merges) (make a second static func if necessary).
#define YYLLOC_DEFAULT(Current, Rhs, N)                 \
    do                                                  \
    {                                                   \
        if ((N))                                        \
        {                                               \
            (Current).begin = YYRHSLOC((Rhs), 1).begin; \
            (Current).end = YYRHSLOC((Rhs), N).end;     \
        }                                               \
        else                                            \
        {                                               \
            (Current).begin = YYRHSLOC((Rhs), 0).end;   \
            (Current).end = YYRHSLOC((Rhs), 0).end;     \
        }                                               \
    } while (0)

#define CLEAR_ERROR(local_semantic_driver) \
    do                                     \
    {                                      \
        local_semantic_driver.recover();   \
        yyerrok;                           \
    } while (0)

#define CLEAR_ERROR_IF_IN_FUNC_PARAM_LIST(local_semantic_driver)              \
    do                                                                        \
    {                                                                         \
        if (local_semantic_driver.context_inspector->is_in_func_param_list()) \
        {                                                                     \
            local_semantic_driver.recover();                                  \
            yyerrok;                                                          \
        }                                                                     \
    } while (0)

#define CLEAR_ERROR_IF_NOT_IN_FORLOOP_CLAUSE(local_semantic_driver)           \
    do                                                                        \
    {                                                                         \
        if (!local_semantic_driver.context_inspector->is_in_forloop_clause()) \
        {                                                                     \
            local_semantic_driver.recover();                                  \
            yyerrok;                                                          \
        }                                                                     \
    } while (0)

#define CLEAR_ERROR_IF_NOT_IN_TABLEDICT(local_semantic_driver)            \
    do                                                                    \
    {                                                                     \
        if (!local_semantic_driver.context_inspector->is_in_table_dict()) \
        {                                                                 \
            local_semantic_driver.recover();                              \
            yyerrok;                                                      \
        }                                                                 \
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
    const alpha::DiagnosticEngine &,
    alpha::DiagnosticReporter &,
    const alpha::SemanticSystem &,
    const std::string &error_message);
#endif // ALPHA_PARSER_PROLOGUE_CODE_HPP
