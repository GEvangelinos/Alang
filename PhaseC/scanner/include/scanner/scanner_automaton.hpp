#ifndef SCANNER_AUTOMATON_HPP
#define SCANNER_AUTOMATON_HPP

#include <array>
#include <core/numeric_types.hpp>

#include "core/translation_unit_buffer.hpp"
#include "parser/alpha_parser.gen.hpp"

namespace alpha
{
class LocationTracker;

class ScannerAutomaton
{
public:
    using LexerReturnType = std::underlying_type_t<decltype(TKN_YYEOF)>;

    // We require at least this much, to avoid eof checks in certain scenarios (like peeking) so we can speed things up.
    static constexpr u64 k_minimum_source_buffer_null_padding = 2;
    static constexpr LexerReturnType TKN_INTERNAL_SKIP = 0x7FFFFFFF;

    ScannerAutomaton(
        LexerCtx& lexer_ctx,
        LocationTracker& lt,
        DiagnosticReporter& dr,
        const TranslationUnitBuffer& tub
    );

    [[nodiscard]] LexerReturnType yield_token(YYSTYPE* yylval, YYLTYPE* yylloc) noexcept;

    [[nodiscard]] const char* last_token_begin() const noexcept;
    [[nodiscard]] const char* last_token_end() const noexcept;
    [[nodiscard]] u64 last_token_length() const noexcept;
    [[nodiscard]] std::string_view last_token_text() const noexcept;
    [[nodiscard]] SourceLocation last_token_location() const noexcept;

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

    enum class OpenerLen : u32
    {
        COMMENT_BLOCK = 2,
        STRING = 1,
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

    static constexpr u32 block_comment_marker_size = 2; // for /*
    static constexpr u32 string_marker_size = 1; // for "

    LexerCtx& lexer_ctx_;
    LocationTracker& lt_;
    DiagnosticReporter& dr_;
    const TranslationUnitBuffer& tub_;
    const char* last_token_begin_; // Points in source_buffer.
    const char* cursor_;           // Points in source_buffer.
    char swap_char_ = '\0';

    template <SrcBuffIdx n>
    [[nodiscard]] char get_nth_char() const noexcept; // Forward only lookup.
    [[nodiscard]] char get_nth_char(SrcBuffIdx n) const noexcept;

    [[nodiscard]] char get_curr_char() const noexcept;
    [[nodiscard]] char get_next_char() const noexcept;
    [[nodiscard]] bool has_reached_eof() const noexcept;


    // By returning the cursor after advancing, we optimize from 2 register lookups to only 1.
    template <SrcBuffIdx n_offset = SrcBuffIdx{1}>
    const char* advance_cursor() noexcept;
    const char* advance_cursor(SrcBuffIdx n) noexcept;

    [[nodiscard]] LexerReturnType register_and_return(
        LexerReturnType token_id, SourceLocation token_loc) noexcept;
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
    [[nodiscard]] LexerReturnType handle_comment_block_nested() noexcept;
    [[nodiscard]] LexerReturnType handle_comment_block_standard() noexcept;

    [[nodiscard]] LexerReturnType handle_alpha_char() noexcept;

    void handle_newline_char() noexcept;
    void handle_invalid_char(char curr_ch) noexcept;
    void register_newline_char() noexcept;

    template <OpenerLen opener_len>
    [[nodiscard]] SourceLocation calculate_opener_loc() const noexcept;
};

inline const char*
ScannerAutomaton::last_token_begin() const noexcept { return last_token_begin_; }

inline const char*
ScannerAutomaton::last_token_end() const noexcept { return cursor_; }

inline u64
ScannerAutomaton::last_token_length() const noexcept
{
    const auto result = last_token_end() - last_token_begin();
    DEBUG_SMART_ASSERT(result > 0 && "A non phony token must have a at least size 1 to exit");
    return result;
}

inline std::string_view
ScannerAutomaton::last_token_text() const noexcept
{
    return std::string_view{last_token_begin(), last_token_length()};
}
} // namespace alpha
#endif // SCANNER_AUTOMATON_HPP
