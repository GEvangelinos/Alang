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
ScannerAutomaton::handle_equal_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '=');
    if (peek_next_char() == '=')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_EQ);
    }
    return register_and_return(TKN_ASSIGN);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_exclamation_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '!');
    if (peek_next_char() == '=') // Only place we support '!', is a part of != token.
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_NEQ);
    }
    dr_.report_invalid_character("!", k_no_loc);
    return ScannerAutomaton::TKN_INTERNAL_SKIP;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_plus_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '+');
    if (peek_next_char() == '+')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_INC);
    }
    return register_and_return(TKN_PLUS);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_minus_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '-');
    if (peek_next_char() == '-')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_DEC);
    }
    return register_and_return(TKN_MINUS);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_left_angle_bracket_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '<');
    if (peek_next_char() == '=')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_LTE);
    }
    return register_and_return(TKN_LT);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_right_angle_bracket_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '>');
    if (peek_next_char() == '=')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_GTE);
    }
    return register_and_return(TKN_GT);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_dot_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '.');
    if (peek_next_char() == '.')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_METHOD_CALL);
    }
    return register_and_return(TKN_DOT);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_colon_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == ':');
    if (peek_next_char() == ':')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_GLOBAL);
    }
    return register_and_return(TKN_COLON);
}


ScannerAutomaton::LexerReturnType
ScannerAutomaton::yield_token()
{
    if (cursor_ == source_size_)
        return TKN_YYEOF;

    while (true)
    {
        LexerReturnType result;
        const char curr_ch = get_curr_char();
        switch (curr_ch)
        { // clang-format off
        case '=': result = handle_equal_char();               break;
        case '!': result = handle_exclamation_char();         break;
        case '+': result = handle_plus_char();                break;
        case '-': result = handle_minus_char();               break;
        case '<': result = handle_left_angle_bracket_char();  break;
        case '>': result = handle_right_angle_bracket_char(); break;
        case '.': result = handle_dot_char();                 break;
        case ':': result = handle_colon_char();               break;
        case '*': return TKN_MUL;
        case '/': return TKN_DIV;
        case '%': return TKN_MOD;
        case '{': return TKN_LEFT_BRACE;
        case '}': return TKN_RIGHT_BRACE;
        case '[': return TKN_LEFT_BRACKET;
        case ']': return TKN_RIGHT_BRACKET;
        case '(': return TKN_LEFT_PAREN;
        case ')': return TKN_RIGHT_PAREN;
        case ';': return TKN_SEMICOLON;
        case ',': return TKN_COMMA;
        case ' ': case '\r': case '\t': case '\v': result = ScannerAutomaton::TKN_INTERNAL_SKIP; break;

        default:
            {
                // if (std::isdigit(curr_ch))

            }
        } // clang-format on
        if (result != ScannerAutomaton::TKN_INTERNAL_SKIP)
            return result;
    }
}
} // namespace alpha
