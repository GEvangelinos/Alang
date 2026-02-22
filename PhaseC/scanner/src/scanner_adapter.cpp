#include "scanner/scanner_adapter.hpp"
#include "core/translation_unit_buffer.hpp"

namespace alpha
{
ScannerAdapter::ScannerAdapter(
    LexerCtx& lexer_ctx,
    LocationTracker& lt,
    DiagnosticReporter& dr,
    const TranslationUnitBuffer& tub)
    : scanner_automaton_(lexer_ctx, lt, dr, tub) {}

#ifdef USE_FLEX_SCANNER
ScannerAdapter::ScannerHandle::ScannerHandle(TranslationUnitBuffer& tu_buffer)
{
    if (alpha_yylex_init(&scanner_) != 0)
        throw std::runtime_error(ATTACH_CONTEXT("Failed to initializing scanner"));

    DEBUG_SMART_ASSERT(!!tu_buffer.data());
    if (alpha_yy_scan_buffer(tu_buffer.data(), tu_buffer.size(), scanner_) == nullptr)
    {
        std::string error =
            "Failed to load Flex buffer. A common cause is forgetting "
            "to append two null bytes for padding in at end of the buffer.";
        if (alpha_yylex_destroy(scanner_) != 0)
            error += " | Additionally, cleanup of the scanner failed.";
        throw std::runtime_error(ATTACH_CONTEXT(error));
    }
}

ScannerAdapter::ScannerHandle::~ScannerHandle()
{
    DEBUG_SMART_ASSERT_EVAL(alpha_yylex_destroy(scanner_) == 0);
}
#endif
} // namespace alpha
