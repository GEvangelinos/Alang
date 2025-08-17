#ifndef ALPHA_SHARED_INTERFACE_HPP
#define ALPHA_SHARED_INTERFACE_HPP

#include "scanner/scanner_context.hpp"

#define ALPHA_YYLEX_SIGNATURE          \
    int alpha_yylex(                   \
        ALPHA_YYSTYPE * yylval_param,        \
        ALPHA_YYLTYPE *yylloc_param,         \
        yyscan_t yyscanner ,           \
        alpha::LocationTracker &lt,    \
        alpha::DiagnosticReporter &dr, \
        alpha::LexerCtx &lexer_ctx)
#endif // ALPHA_SHARED_INTERFACE_HPP