#ifndef ALPHA_SHARED_INTERFACE_HPP
#define ALPHA_SHARED_INTERFACE_HPP

#include "scanner/alpha_scanner_context.hpp"

#define ALPHA_YYLEX_SIGNATURE int alpha_yylex(Alpha::LexerCtx &scnr_ctx, Alpha::ErrorTracker &et)

#endif /* ALPHA_SHARED_INTERFACE_HPP*/