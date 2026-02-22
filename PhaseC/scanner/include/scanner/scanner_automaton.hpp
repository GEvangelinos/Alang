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
    using LexerReturnType = std::underlying_type_t<decltype(TKN_YYEOF)>;

    // We require at least this much, to avoid eof checks in certain scenarios (like peeking) so we can speed things up.
    static constexpr u64 k_minimum_source_buffer_null_padding = 2;
    static constexpr LexerReturnType TKN_INTERNAL_SKIP = static_cast<LexerReturnType>(0x7FFFFFFF);

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
    enum class KeywordId : i8
    {
        IF,
        ELSE,
        WHILE,
        FOR,
        FUNCTION,
        RETURN,
        BREAK,
        CONTINUE,
        NOT,
        AND,
        OR,
        LOCAL,
        TRUE,
        FALSE,
        NIL,
        COUNT_,         // Note a keyword
        NONE_ = COUNT_, // Not a keyword (used for internal purposes)
    };

    struct KeywordToken
    {
        const std::string_view name;
        const LexerReturnType bison_token_id;
        DEBUG(KeywordId id;)
    };

    static constexpr std::array keyword_names_and_tokens_
    { // clang-format off
        KeywordToken{"if"      , TKN_IF      , DEBUG(KeywordId::IF)},
        KeywordToken{"else"    , TKN_ELSE    , DEBUG(KeywordId::ELSE)},
        KeywordToken{"while"   , TKN_WHILE   , DEBUG(KeywordId::WHILE)},
        KeywordToken{"for"     , TKN_FOR     , DEBUG(KeywordId::FOR)},
        KeywordToken{"function", TKN_FUNCTION, DEBUG(KeywordId::FUNCTION)},
        KeywordToken{"return"  , TKN_RETURN  , DEBUG(KeywordId::RETURN)},
        KeywordToken{"break"   , TKN_BREAK   , DEBUG(KeywordId::BREAK)},
        KeywordToken{"continue", TKN_CONTINUE, DEBUG(KeywordId::CONTINUE)},
        KeywordToken{"not"     , TKN_NOT     , DEBUG(KeywordId::NOT)},
        KeywordToken{"and"     , TKN_AND     , DEBUG(KeywordId::AND)},
        KeywordToken{"or"      , TKN_OR      , DEBUG(KeywordId::OR)},
        KeywordToken{"local"   , TKN_LOCAL   , DEBUG(KeywordId::LOCAL)},
        KeywordToken{"true"    , TKN_TRUE    , DEBUG(KeywordId::TRUE)},
        KeywordToken{"false"   , TKN_FALSE   , DEBUG(KeywordId::FALSE)},
        KeywordToken{"nil"     , TKN_NIL     , DEBUG(KeywordId::NIL)},
    }; // clang-format on

    LexerCtx& lexer_ctx_;
    LocationTracker& lt_;
    DiagnosticReporter& dr_;
    const char* const source_buffer_ = nullptr;
    const u64 source_size_ = 0;
    const u64 source_buffer_null_padding_ = 0;
    u64 cursor_ = 0; // Index based (points on source_buffer)

    template <u64 n>
    [[nodiscard]] char get_nth_char() const noexcept; // Forward only lookup.
    [[nodiscard]] char get_nth_char(u64 n) const noexcept;

    [[nodiscard]] const char* get_cursor_address() const noexcept;
    [[nodiscard]] char get_curr_char() const noexcept;
    [[nodiscard]] char get_next_char() const noexcept;
    [[nodiscard]] bool has_reached_eof() const noexcept;

    template <u64 n = 1>
    void advance_cursor() noexcept;
    void advance_cursor(u64 n) noexcept;

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

    [[nodiscard]] LexerReturnType handle_alpha_char() noexcept;

    void register_newline_char() noexcept;
};
} // namespace alpha
#endif // SCANNER_AUTOMATON_HPP
