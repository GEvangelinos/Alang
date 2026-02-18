#ifndef SCANNER_AUTOMATON_HPP
#define SCANNER_AUTOMATON_HPP

#include <array>
#include <core/numeric_types.hpp>

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
    static inline u64 k_minimum_source_buffer_null_padding = 2;

    ScannerAutomaton(
        LexerCtx& lexer_ctx,
        DiagnosticReporter& dr,
        const char* source_buffer,
        u64 source_size,
        u64 source_buffer_null_padding
    );

    [[nodiscard]] LexerReturnType yield_token(LexerCtx& lexer_ctx);

private:
    LexerCtx& lexer_ctx_;
    DiagnosticReporter& dr_;
    const char* const source_buffer_ = nullptr;
    const u64 source_size_ = 0;
    const u64 source_buffer_null_padding_ = 0;
    u64 cursor_ = 0; // Index based (points on source_buffer)

    [[nodiscard]] bool has_reached_eof() const noexcept;
    [[nodiscard]] char get_curr_char() noexcept;
    [[nodiscard]] char peek_next_char() noexcept;
    void advance_cursor() noexcept { ++cursor_; }
    [[nodiscard]] LexerReturnType register_and_return(LexerReturnType token_id) noexcept;
    [[nodiscard]] LexerReturnType handle_equal_char() noexcept;
    [[nodiscard]] LexerReturnType handle_exclamation_char() noexcept;
    [[nodiscard]] LexerReturnType handle_plus_sign() noexcept;
};
} // namespace alpha
#endif // SCANNER_AUTOMATON_HPP
