#ifndef ALPHA_SHARED_INTERFACE_HPP
#define ALPHA_SHARED_INTERFACE_HPP

#include "scanner/alpha_scanner_context.hpp"

#define ALPHA_YYLEX_SIGNATURE int alpha_yylex( \
    Alpha::LexerCtx &lexer_ctx,                 \
    Alpha::ErrorTracker &et,                   \
    Alpha::LocationTracker &lt)

#endif // ALPHA_SHARED_INTERFACE_HPP