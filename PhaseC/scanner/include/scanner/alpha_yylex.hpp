#ifndef ALPHA_YYLEX_HPP
#define ALPHA_YYLEX_HPP

#include "scanner/scanner_adapter.hpp"

/* THIS IS THE SYMBOL BISON LOOKS FOR */
[[nodiscard]] static int alpha_yylex(
    YYSTYPE* yylval_param,
    YYLTYPE* yylloc_param,
    alpha::LexerCtx& lexer_ctx,
    alpha::LocationTracker& lt,
    alpha::DiagnosticReporter& dr,
    alpha::ScannerAdapter& scanner_adapter)
{
    return scanner_adapter.alpha_yylex(yylval_param, yylloc_param, lexer_ctx, lt, dr);
}
#endif //ALPHA_YYLEX_HPP
