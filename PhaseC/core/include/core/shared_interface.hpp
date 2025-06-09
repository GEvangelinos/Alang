#ifndef ALPHA_SHARED_INTERFACE_HPP
#define ALPHA_SHARED_INTERFACE_HPP

#include "scanner/scanner_context.hpp"

#define ALPHA_YYLEX_SIGNATURE int alpha_yylex(  \
    Alpha::LocationTracker &lt,                 \
    Alpha::DiagnosticEngine &diagnostic_engine, \
    Alpha::LexerCtx &lexer_ctx)

#endif // ALPHA_SHARED_INTERFACE_HPP