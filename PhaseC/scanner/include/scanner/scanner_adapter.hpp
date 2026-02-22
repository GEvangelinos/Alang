#ifndef SCANNER_ADAPTER_HPP
#define SCANNER_ADAPTER_HPP

#ifdef USE_FLEX_SCANNER
#else
#include "scanner_automaton.hpp"
#endif

#include <alpha_parser.gen.hpp>
#include <scanner/alpha_scanner.gen.hpp>


#ifdef USE_FLEX_SCANNER
// Forward declaration instead of including the <parser/alpha_parser.gen.hpp> header
typedef void* yyscan_t;

namespace alpha
{
class ScannerAdapter
{
public:
    ScannerAdapter(
        LexerCtx&,
        LocationTracker&,
        DiagnosticReporter&,
        TranslationUnitBuffer& tub
    ): scanner_handle_(tub) {}

    [[nodiscard]] int alpha_yylex(
        YYSTYPE* yylval_param,
        YYLTYPE* yylloc_param,
        alpha::LexerCtx& lexer_ctx,
        alpha::LocationTracker& lt,
        alpha::DiagnosticReporter& dr)
    {
        return ::alpha_yylex(yylval_param, yylloc_param, scanner_handle_.get(), lexer_ctx, lt,dr);
    }

private:
    class ScannerHandle : private alpha::Immobile
    {
    public:
        ScannerHandle() = delete;
        explicit ScannerHandle(TranslationUnitBuffer& tu_buffer);
        ~ScannerHandle();

        [[nodiscard]] yyscan_t get() const noexcept { return scanner_; }

    private:
        yyscan_t scanner_;
    };

    ScannerHandle scanner_handle_;
};
} // namespace alpha
#else
namespace alpha
{
class TranslationUnitBuffer;

class ScannerAdapter
{
public:
    ScannerAdapter(
        LexerCtx& lexer_ctx,
        LocationTracker& lt,
        DiagnosticReporter& dr,
        const TranslationUnitBuffer& tub);

    [[nodiscard]] int alpha_yylex(
        YYSTYPE*,
        YYLTYPE*,
        alpha::LexerCtx&,
        alpha::LocationTracker&,
        alpha::DiagnosticReporter&) { return scanner_automaton_.yield_token(); }

    [[nodiscard]] std::string_view
    last_token_text() const noexcept { return scanner_automaton_.last_token_text(); }
    [[nodiscard]] u64
    last_token_length() const noexcept { return scanner_automaton_.last_token_length(); }

private:
    ScannerAutomaton scanner_automaton_;
};

/* THIS IS THE SYMBOL BISON LOOKS FOR */
[[nodiscard]] static int alpha_yylex(
    YYSTYPE* yylval_param,
    YYLTYPE* yylloc_param,
    alpha::LexerCtx& lexer_ctx,
    alpha::LocationTracker& lt,
    alpha::DiagnosticReporter& dr,
    ScannerAdapter& scanner_adapter)
{
    return scanner_adapter.alpha_yylex(yylval_param, yylloc_param, lexer_ctx, lt, dr);
}
} // namespace alpha
#endif
#endif // SCANNER_ADAPTER_HPP
