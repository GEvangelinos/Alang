#include <alpha_parser.gen.hpp>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <scanner/scanner_automaton.hpp>

#include "scanner_prologue.hpp"
#include "scanner/scanner_context.hpp"
#include "support/misc_tools.hpp"


namespace alpha
{
ScannerAutomaton::ScannerAutomaton(
    LexerCtx& lexer_ctx,
    DiagnosticReporter& dr,
    const char* const source_buffer,
    const u64 source_size,
    const u64 source_buffer_null_padding)
    : lexer_ctx_(lexer_ctx),
      dr_(dr),
      source_buffer_(support::require_ptr(source_buffer)),
      source_size_(source_size),
      source_buffer_null_padding_(source_buffer_null_padding)
{
    if (source_buffer_null_padding_ < ScannerAutomaton::k_minimum_source_buffer_null_padding)
        throw std::logic_error("Insufficient padding detected");
    for (u64 pad_index = 0; pad_index < source_buffer_null_padding_; ++pad_index)
        if (source_buffer_[source_size_ + pad_index] != '\0')
            throw std::logic_error("Critical sentinel corruption detected");
}

char
ScannerAutomaton::get_curr_char() noexcept
{
    DEBUG_SMART_ASSERT(!has_reached_eof());
    return source_buffer_[cursor_];
}

char
ScannerAutomaton::peek_next_char() noexcept
{
    DEBUG_SMART_ASSERT(!has_reached_eof());
    return source_buffer_[cursor_ + 1];
}


bool
ScannerAutomaton::has_reached_eof() const noexcept { return cursor_ == source_size_; }

ScannerAutomaton::LexerReturnType
ScannerAutomaton::register_and_return(const LexerReturnType token_id) noexcept
{
    lexer_ctx_.register_token(alpha::TokenInfo{
        .id = static_cast<alpha_yytoken_kind_t>(token_id),
        .loc = k_no_loc
    });
    return token_id;
}


ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_equal_sign() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '=');
    if (peek_next_char() != '=')
        return register_and_return(TKN_ASSIGN);
    advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
    return register_and_return(TKN_EQ);
}


ScannerAutomaton::LexerReturnType
ScannerAutomaton::yield_token(LexerCtx& lexer_ctx)
{
    if (cursor_ == source_size_)
        return TKN_YYEOF;

    while (true)
    {
        const char curr_ch = get_next_char();
        switch (curr_ch)
        {
        case '=': return handle_equal_sign();
        case '+': YIELD_TOKEN_CUSTOM(TKN_PLUS);
        }
    }
}
} // namespace alpha
