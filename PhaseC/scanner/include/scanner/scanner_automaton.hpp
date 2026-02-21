#ifndef SCANNER_AUTOMATON_HPP
#define SCANNER_AUTOMATON_HPP

#include <array>
#include <core/numeric_types.hpp>

#include "support/misc_tools.hpp"

namespace alpha
{
class ScannerAutomaton
{
public:
    using LexerReturnType = int;

    struct Token
    {
        const char* const name;
        const int bison_token_id;
    };

    static inline std::array k_keyword_pool{
        Token{"if", TKN_IF},
        Token{"else", TKN_ELSE},
        Token{"while", TKN_WHILE},
        Token{"for", TKN_FOR},
        Token{"function", TKN_FUNCTION},
        Token{"return", TKN_RETURN},
        Token{"break", TKN_BREAK},
        Token{"continue", TKN_CONTINUE},
        Token{"not", TKN_NOT},
        Token{"and", TKN_AND},
        Token{"or", TKN_OR},
        Token{"local", TKN_LOCAL},
        Token{"true", TKN_TRUE},
        Token{"false", TKN_FALSE},
        Token{"nil", TKN_NIL},
    };

    // We require at least this much, to avoid eof checks in certain scenarios (like peeking) so we can speed things up.
    static constexpr u64 k_minimum_source_buffer_null_padding = 2;
    static constexpr LexerReturnType TKN_INTERNAL_SKIP = 0x7FFFFFFF;

    ScannerAutomaton(
        LexerCtx& lexer_ctx,
        LocationTracker& lt,
        DiagnosticReporter& dr,
        const char* source_buffer,
        u64 source_size,
        u64 source_buffer_null_padding
    );

    [[nodiscard]] LexerReturnType yield_token() noexcept;

private:
    LexerCtx& lexer_ctx_;
    LocationTracker& lt_;
    DiagnosticReporter& dr_;
    const char* const source_buffer_ = nullptr;
    const u64 source_size_ = 0;
    const u64 source_buffer_null_padding_ = 0;
    u64 cursor_ = 0; // Index based (points on source_buffer)

    template <u64 n>
    [[nodiscard]] char get_nth_char() noexcept; // Forward only lookup.

    [[nodiscard]] char get_curr_char() noexcept;
    [[nodiscard]] char get_next_char() noexcept;
    [[nodiscard]] bool has_reached_eof() const noexcept;

    template <u64 n = 1>
    void advance_cursor() noexcept;

    [[nodiscard]] LexerReturnType register_and_return(LexerReturnType token_id) noexcept;
    [[nodiscard]] LexerReturnType handle_equal_char() noexcept;
    [[nodiscard]] LexerReturnType handle_exclamation_char() noexcept;
    [[nodiscard]] LexerReturnType handle_plus_char() noexcept;
    [[nodiscard]] LexerReturnType handle_minus_char() noexcept;
    [[nodiscard]] LexerReturnType handle_left_angle_bracket_char() noexcept;
    [[nodiscard]] LexerReturnType handle_right_angle_bracket_char() noexcept;
    [[nodiscard]] LexerReturnType handle_dot_char() noexcept;
    [[nodiscard]] LexerReturnType handle_colon_char() noexcept;
    [[nodiscard]] LexerReturnType handle_double_quote_char() noexcept;

    [[nodiscard]] LexerReturnType handle_number_char() noexcept;
    [[nodiscard]] LexerReturnType handle_hex_number() noexcept;
    [[nodiscard]] LexerReturnType handle_decimal_number() noexcept;
    [[nodiscard]] LexerReturnType handle_float_number() noexcept;

    [[nodiscard]] LexerReturnType handle_slash_char() noexcept;
    [[nodiscard]] LexerReturnType handle_comment_line() noexcept;
    [[nodiscard]] LexerReturnType handle_comment_block() noexcept;

    void register_newline_char() noexcept;
};

template <u64 n>
char
ScannerAutomaton::get_nth_char() noexcept
{
    const auto index = cursor_ + n;
    DEBUG_SMART_ASSERT(index < source_size_ + source_buffer_null_padding_ && "Illegal access");
    return DEBUG_REQUIRE_PTR(source_buffer_)[index];
}

template <u64 n = 1>
void
ScannerAutomaton::advance_cursor() noexcept
{
    DEBUG_SMART_ASSERT(cursor_ < source_size_); // Is OK before.
    cursor_ += n;
    DEBUG_SMART_ASSERT(cursor_ < source_size_); // Is OK after.
}
} // namespace alpha
#endif // SCANNER_AUTOMATON_HPP
