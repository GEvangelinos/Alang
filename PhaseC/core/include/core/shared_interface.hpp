#ifndef ALPHA_SHARED_INTERFACE_HPP
#define ALPHA_SHARED_INTERFACE_HPP

// Forward declarations required by alpha_yylex's signature
namespace alpha
{
class LocationTracker;
class Lexer;
class DiagnosticReporter;
} // namespace alpha

#ifdef YY_DECL
    #error "YYD_DECL is already defined at the place you included the header."
#else
    #define YY_DECL          \
        int alpha_yylex(                   \
            YYSTYPE * yylval_param,        \
            YYLTYPE *yylloc_param,         \
            yyscan_t yyscanner ,           \
            alpha::LexerCtx &lexer_ctx,    \
            alpha::LocationTracker &lt,    \
            alpha::DiagnosticReporter &dr)
#endif

#endif // ALPHA_SHARED_INTERFACE_HPP
