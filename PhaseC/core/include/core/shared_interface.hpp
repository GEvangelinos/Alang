#ifndef ALPHA_SHARED_INTERFACE_HPP
#define ALPHA_SHARED_INTERFACE_HPP

#include "scanner/scanner_context.hpp"

#define ALPHA_YYLEX_SIGNATURE int alpha_yylex(  \
    alpha::LocationTracker &lt,                 \
    alpha::DiagnosticReporter &dr, \
    alpha::LexerCtx &lexer_ctx)

#endif // ALPHA_SHARED_INTERFACE_HPP