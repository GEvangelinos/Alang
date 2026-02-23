#ifndef SCANNER_ADAPTER_HPP
#define SCANNER_ADAPTER_HPP


#include <string_view>
#include "core/numeric_types.hpp"

// clang-format off
namespace alpha{ class TranslationUnitBuffer; }
// clang-format on

// Conditional #include(s):
#ifdef USE_FLEX_SCANNER
#include "core/basics.hpp"

    #ifdef YY_DECL
    #error "Flex has already defined YY_DECL (automatically)"
    #else
        #define YY_DECL                     \
            int alpha_yylex(                \
                YYSTYPE * yylval_param,     \
                YYLTYPE *yylloc_param,      \
                yyscan_t yyscanner ,        \
                alpha::LexerCtx &lexer_ctx, \
                alpha::LocationTracker &lt, \
                alpha::DiagnosticReporter &dr)
    #endif

    #include "parser/alpha_parser.gen.hpp"
    YY_DECL;
    #include <scanner/alpha_scanner.gen.hpp>

    #ifdef YY_DECL_IS_OURS
    #error "Flex definned his (automatically)"
    #endif
#else
    #include "scanner_automaton.hpp"
#endif

// Conditional FWD(s):
#ifdef USE_FLEX_SCANNER
// clang-format off
namespace alpha{ class DiagnosticReporter; }
namespace alpha{ class LocationTracker; }
namespace alpha{ class LexerCtx; }
// clang-format on
#else
#endif

namespace alpha
{
class ScannerAdapter
{
public:
    ScannerAdapter(
        [[maybe_unused]] LexerCtx& lexer_ctx,
        [[maybe_unused]] LocationTracker& lt,
        [[maybe_unused]] DiagnosticReporter& dr,
        [[maybe_unused]] TranslationUnitBuffer& tub
    );

    [[nodiscard]] int alpha_yylex(
        [[maybe_unused]] YYSTYPE* yylval_param,
        [[maybe_unused]] YYLTYPE* yylloc_param,
        [[maybe_unused]] alpha::LexerCtx& lexer_ctx,
        [[maybe_unused]] alpha::LocationTracker& lt,
        [[maybe_unused]] alpha::DiagnosticReporter& dr)
    {
        #ifdef USE_FLEX_SCANNER
        return ::alpha_yylex(yylval_param, yylloc_param, scanner_handle_.get(), lexer_ctx, lt, dr);
        #else
        return scanner_automaton_.yield_token(yylval_param, yylloc_param);
        #endif
    }

    [[nodiscard]] std::string_view
    last_token_text() const noexcept
    {
        #ifdef USE_FLEX_SCANNER
        return alpha_yyget_text(scanner_handle_.get());
        #else
        return scanner_automaton_.last_token_text();
        #endif
    }

private:
    #ifdef USE_FLEX_SCANNER
    class ScannerHandle : private Immobile
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
    #else
    ScannerAutomaton scanner_automaton_;
    #endif
};
} // namespace alpha
#endif // SCANNER_ADAPTER_HPP
