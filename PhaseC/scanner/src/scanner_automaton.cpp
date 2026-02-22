#include <alpha_parser.gen.hpp>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <scanner/scanner_automaton.hpp>

#include "scanner_prologue.hpp"
#include "scanner/scanner_context.hpp"
#include "support/misc_tools.hpp"
#include "support/string_tools.hpp"

namespace
{
constexpr auto is_id_body_char = [](const unsigned char c) consteval
{
    return alpha::support::is_digit(c) || alpha::support::is_alpha(c) || c == '_';
};

constexpr std::array<bool, 256> g_id_body_table =
    []() consteval
    {
        std::array<bool, 256> result;
        for (unsigned short uc = 0; uc <= std::numeric_limits<unsigned char>::max(); ++uc)
            result[uc] = is_id_body_char(uc);
        return result;
    }();

template <unsigned char uc>
struct report_id_body_table_mismatch; // NEVER DEFINE (Whole point of this, is it errors upon use)

// Because Templates are instantiated before code is executed (even at compile time context)
// the only way to find at which position g_id_body_table is off, is to use recursive template instantiation
// --- Note this assertion is no longer needed... As I now initialize the table algorithmically  --- //
// --- and not manually (filling bool table with 0s and 1s) but it's still a smart piece of code --- //
// --- so I am keeping it, as it took me lots of tinkering to make the recursive assertion work. --- //
template <unsigned short Index>
consteval bool assert_id_body_table_integrity()
{
    // Check for false positives:
    if constexpr (Index > std::numeric_limits<unsigned char>::max())
        return true;
    else if constexpr (!is_id_body_char(Index) && g_id_body_table[Index]) // False Positives
        report_id_body_table_mismatch<Index>{};
    else if constexpr (is_id_body_char(Index) && !g_id_body_table[Index]) // False Negatives
        report_id_body_table_mismatch<Index>{};
    else
        return assert_id_body_table_integrity<Index + 1>();
}

static_assert(assert_id_body_table_integrity<0>(),
              "ID Table mismatch! See compiler output for index.");
} // namespace

namespace alpha
{
ScannerAutomaton::ScannerAutomaton(
    LexerCtx& lexer_ctx,
    LocationTracker& lt,
    DiagnosticReporter& dr,
    const char* const source_buffer,
    const u64 source_size,
    const u64 source_buffer_null_padding)
    : lexer_ctx_(lexer_ctx),
      lt_(lt),
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

    // Compile Time Evaluation only code.
    static_assert(
        keyword_names_and_tokens_.size() == static_cast<u64>(KeywordId::COUNT_),
        "Keyword collection size mismatch: the 'keyword_names_and_tokens_' array must have exactly "
        "'KEYWORD_COUNT_' elements. Did you add a new KeywordId without adding its metadata?"
    );
#ifdef DEBUG_MODE
    static_assert(
        []()
        {
            using UT = std::underlying_type_t<KeywordId>;
            for (UT i = 0; i < static_cast<UT>(KeywordId::COUNT_); ++i)
                if (static_cast<KeywordId>(i) != keyword_names_and_tokens_[i].id)
                    return false;
            return true;
        }(),
        "Keyword ID ordering violation: The 'id' field of each KeywordToken must match "
        "its position in the 'keyword_names_and_tokens_' array. Ensure the array order "
        "strictly follows the 'KeywordId' enum definition."
    );
#endif // DEBUG_MODE
}

template <u64 n>
char
ScannerAutomaton::get_nth_char() const noexcept
{
    const auto index = cursor_ + n;
    DEBUG_SMART_ASSERT(index < source_size_ + source_buffer_null_padding_ && "Illegal access");
    const auto result = DEBUG_REQUIRE_PTR(source_buffer_)[index];
    DEBUG_SMART_ASSERT(
        result >= std::numeric_limits<unsigned char>::min(),
        result <= std::numeric_limits<char>::max()
    );
    return result;
}

char
ScannerAutomaton::get_nth_char(const u64 n) const noexcept
{
    const auto index = cursor_ + n;
    DEBUG_SMART_ASSERT(index < source_size_ + source_buffer_null_padding_ && "Illegal access");
    return DEBUG_REQUIRE_PTR(source_buffer_)[index];
}

template <u64 n>
void
ScannerAutomaton::advance_cursor() noexcept
{
    static_assert(n > 0, "Why advance by zero?");
    DEBUG_SMART_ASSERT(cursor_ < source_size_); // Is OK before?
    cursor_ += n;
    // After: Can be AT source_size_ (EOF), but not past it.
    DEBUG_SMART_ASSERT(cursor_ <= source_size_); // Is OK after?
}

void
ScannerAutomaton::advance_cursor(const u64 n) noexcept
{
    DEBUG_SMART_ASSERT(cursor_ < source_size_, n > 0 && "why advance by zero?"); // Is OK before?
    cursor_ += n;
    // After: Can be AT source_size_ (EOF), but not past it.
    DEBUG_SMART_ASSERT(cursor_ <= source_size_); // Is OK after?
}

const char*
ScannerAutomaton::get_cursor_address() const noexcept { return source_buffer_ + cursor_; }

char
ScannerAutomaton::get_curr_char() const noexcept
{
    DEBUG_SMART_ASSERT(!has_reached_eof());
    return get_nth_char<0>();
}

char
ScannerAutomaton::get_next_char() const noexcept
{
    DEBUG_SMART_ASSERT(!has_reached_eof());
    return get_nth_char<1>();
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
    if (get_next_char() == '=')
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
    if (get_next_char() == '=') // Only place we support '!', is a part of != token.
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
    if (get_next_char() == '+')
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
    if (get_next_char() == '-')
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
    if (get_next_char() == '=')
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
    if (get_next_char() == '=')
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
    const char next_ch = get_next_char();
    if (support::is_digit(next_ch))
        return handle_float_number();
    if (next_ch == '.')
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
    if (get_next_char() == ':')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_GLOBAL);
    }
    return register_and_return(TKN_COLON);
}

void
ScannerAutomaton::register_newline_char() noexcept { lt_.append_line(lexer_ctx_.source_index); }


ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_comment_line() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '/', get_next_char() == '/');
    while (!has_reached_eof() && get_next_char() != '\n')
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
    return ScannerAutomaton::TKN_INTERNAL_SKIP;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_comment_block() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '/', get_next_char() == '*'); // Still at beginning.
    u64 block_comment_depth = 0;
    while (!has_reached_eof())
    {
        // Reminder even if ch2 is after last valid CHAR (so its after EOF), its still valid due to padding requirement (in ctor)
        const char ch1 = get_curr_char();
        const char ch2 = get_next_char();
        if (ch1 == '\n')
            register_newline_char();

        if (ch1 == '/' && ch2 == '*')
        {
            advance_cursor(); // consume '/' now, consume '*' at end.
            ++block_comment_depth;
        }
        else if (ch1 == '*' && ch2 == '/')
        {
            advance_cursor(); // consume '*' now, consume '/' at end.
            --block_comment_depth;
        }

        if (block_comment_depth == 0)
            return ScannerAutomaton::TKN_INTERNAL_SKIP;
        advance_cursor(); // Only advance if not on final closing '/'. (for caller to consume)
    }
    return TKN_YYEOF;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_slash_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '/');
    const char next_ch = get_next_char();
    if (next_ch == '/')
        return handle_comment_line();
    if (next_ch == '*')
        return handle_comment_block();
    return register_and_return(TKN_DIV);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_double_quote_char() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '\"');
    // Just started handling string, so we first things first consume the initialization marker.
    advance_cursor();
    while (!has_reached_eof())
    {
        // Reminder even if ch2 is after last valid CHAR (so its after EOF), its still valid due to padding requirement (in ctor)
        const char ch1 = get_curr_char();
        if (ch1 == '\"')       // Handle matching "
            return TKN_STRING; // Let Main dispatcher consume the matching " char.
        if (ch1 == '\\')       // Handle potential escape code
        {
            const char ch2 = get_next_char();
            switch (ch2)
            {
            case 'n':
            case 'r':
            case 't':
            case 'v':
            case 'f':
            case '\\':
            case '\"':
                advance_cursor(); // Consume ch1 here, and at end consume ch2.
                DEBUG_SMART_ASSERT(!has_reached_eof() && "If ch2 past EOF, ch2 must be NULL-byte");
                break;
            default: break; // Do nothing
            }
        }
        advance_cursor();
    }
    return TKN_YYEOF;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_hex_number() noexcept
{
    DEBUG_SMART_ASSERT(
        get_nth_char<0>() == '0',
        get_nth_char<1>() == 'x' || get_nth_char<1>() == 'X',
        support::is_xdigit(get_nth_char<2>())
    );
    advance_cursor<2>(); // We consume 0 and 'x'
    while (const char next_ch = get_next_char())
    {
        DEBUG_SMART_ASSERT(
            !has_reached_eof() &&
            "To reach EOF here it would mean curr is at EOF (so NULL-BYTE) "
            "and that also means next_ch on previous iterations was NULL-BYTE "
            "and pass the isxdigit() check and advance cursor with a NULL-BYTE"
        );
        if (support::is_xdigit(next_ch))
            advance_cursor();
    }
    DEBUG_SMART_ASSERT(support::is_xdigit(get_curr_char()));
    return TKN_INT;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_float_number() noexcept
{
    DEBUG_SMART_ASSERT(get_curr_char() == '.', support::is_digit(get_next_char()));
    advance_cursor(); // To consume '.'

    // --- Prefix digits consumption loop --- //
    while (support::is_digit(get_next_char()))
        advance_cursor();
    DEBUG_SMART_ASSERT(!has_reached_eof(), support::is_digit(get_curr_char()));

    // --- Handle potential scientific floats --- //
    const char next_ch = get_next_char();
    DEBUG_SMART_ASSERT(!support::is_digit(next_ch) && "Prefix digit loop should consume that");
    if (next_ch == 'e' || next_ch == 'E') // Handle Scientific floats
    {
        static_assert(ScannerAutomaton::k_minimum_source_buffer_null_padding >= 1);
        DEBUG_SMART_ASSERT(source_buffer_null_padding_ >= 1);
        const char next_next_ch = get_nth_char<2>(); // Valid cause <1> was non null-byte.
        if (next_next_ch == '+' || next_next_ch == '-')
        {
            static_assert(ScannerAutomaton::k_minimum_source_buffer_null_padding >= 2);
            DEBUG_SMART_ASSERT(source_buffer_null_padding_ >= 2);
            const char next_next_next_ch = get_nth_char<3>();
            if (support::is_digit(next_next_next_ch))
            {
                DEBUG_SMART_ASSERT(
                    support::is_digit(get_nth_char<0>()),
                    get_nth_char<1>() == 'e' || get_nth_char<1>() == 'E',
                    get_nth_char<2>() == '+' || get_nth_char<2>() == '-',
                    support::is_digit(get_nth_char<3>())
                );
                advance_cursor<3>(); // Consume digit before E, consume E, consume +-
            }
            // else "This case is for something like 1.2e+myid, which is not scientific notation ... just a `1.2` float"
        }
        else if (support::is_digit(next_next_ch))
        {
            DEBUG_SMART_ASSERT(
                support::is_digit(get_nth_char<0>()),
                get_nth_char<1>() == 'e' || get_nth_char<1>() == 'E',
                support::is_digit(get_nth_char<2>())
            );
            advance_cursor<2>(); // Consume digit before E and consume E.
        }
        // else "next_ch was e or E but without digit suffix e or E is just an ID fragment, not part of float"
    }
    DEBUG_SMART_ASSERT(support::is_digit(get_curr_char()));

    // --- Suffix digits consumption loop --- //
    while (support::is_digit(get_next_char()))
        advance_cursor();
    DEBUG_SMART_ASSERT(!has_reached_eof(), support::is_digit(get_curr_char()));

    // Last digit is consumed by main dispatch loop (our caller).
    return TKN_FLOAT;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_decimal_number() noexcept
{
    DEBUG_SMART_ASSERT(support::is_digit(get_curr_char()));
    while (true)
    {
        DEBUG_SMART_ASSERT(!has_reached_eof());
        const char next_ch = get_next_char();
        if (next_ch == '.' && support::is_digit(get_nth_char<2>()))
        {
            advance_cursor();
            return handle_float_number();
        }
        if (support::is_digit(next_ch))
            advance_cursor();
        else
            break;
    }
    DEBUG_SMART_ASSERT(support::is_digit(get_curr_char()), !support::is_digit(get_next_char()));
    return TKN_INT;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_number_char() noexcept
{
    const char curr_ch = get_curr_char();
    DEBUG_SMART_ASSERT(support::is_digit(curr_ch));

    // For HEX nums we require at least a hex digit after 0x as without it, 0x is just a decimal 0 and an id x.
    if (curr_ch == '0')
    {
        const char next_ch = get_next_char();
        if ((next_ch == 'x' || next_ch == 'X') && support::is_xdigit(get_nth_char<2>()))
            return handle_hex_number();
    }

    // If here then the number isn't HEX, So we need to check if integer or float.
    return handle_decimal_number(); // This functions also handle floats starting with decimals.

    // Note float without decimal (ex: `.15`) are not detected here (but most likely on '.' handler).
}

#define CASE_LIST_FOR_SPACES  \
    case ' ': case '\r': case '\t': case '\v'
#define CASE_LIST_FOR_NUMBERS \
    case '0': case '1': case '2': case '3': case '4': \
    case '5': case '6': case '7': case '8': case '9'
#define CASE_LIST_FOR_LETTERS \
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': \
    case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': \
    case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':           \
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': \
    case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': \
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z'

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_alpha_char() noexcept
{
    DEBUG_SMART_ASSERT(support::is_alpha(get_curr_char()));
    u64 word_length = 1;
    while (g_id_body_table[get_nth_char(word_length)])
        ++word_length;

    DEBUG_SMART_ASSERT(g_id_body_table[get_curr_char()]);

    const auto find_possible_keyword = [this, word_length]() -> KeywordId
    {
        const char ch0 = get_nth_char<0>();
        switch (word_length)
        {
        case 2: // IF | OR
            if (ch0 == 'i') return KeywordId::IF;
            if (ch0 == 'o') return KeywordId::OR;
            break;
        case 3: // AND | FOR | NOT | NIL
            if (ch0 == 'a') return KeywordId::AND;
            if (ch0 == 'f') return KeywordId::FOR;
            if (ch0 == 'n')
            {
                const char ch1 = get_nth_char<1>();
                if (ch1 == 'o') return KeywordId::NOT;
                if (ch1 == 'i') return KeywordId::NIL;
            }
            break;
        case 4: // ELSE | TRUE
            if (ch0 == 'e') return KeywordId::ELSE;
            if (ch0 == 't') return KeywordId::TRUE;
            break;
        case 5: // BREAK | FALSE | LOCAL | WHILE
            if (ch0 == 'b') return KeywordId::BREAK;
            if (ch0 == 'f') return KeywordId::FALSE;
            if (ch0 == 'l') return KeywordId::LOCAL;
            if (ch0 == 'w') return KeywordId::WHILE;
            break;
        case 6: // RETURN
            if (ch0 == 'r') return KeywordId::RETURN;
            break;
        case 8: // CONTINUE | FUNCTION
            if (ch0 == 'c') return KeywordId::CONTINUE;
            if (ch0 == 'f') return KeywordId::FUNCTION;
            break;
        default: break;
        }
        return KeywordId::NONE_;
    };

    const KeywordId possible_keyword = find_possible_keyword();
    LexerReturnType result_token = TKN_ID;
    if (possible_keyword != KeywordId::NONE_)
    {
        const auto keyword_idx = static_cast<std::underlying_type_t<KeywordId>>(possible_keyword);
        const KeywordToken expected_token = keyword_names_and_tokens_[keyword_idx];
        if (std::string_view{get_cursor_address(), word_length} == expected_token.name)
            result_token = expected_token.bison_token_id;
    }

    advance_cursor(word_length - 1); // Minus 1 cause we want to not consume last valid char.
    DEBUG_SMART_ASSERT(
        g_id_body_table[get_curr_char()] && "last id fragment consumed by caller",
        !g_id_body_table[get_next_char()]
        );
    return result_token;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::yield_token() noexcept
{
    if (cursor_ == source_size_)
        return TKN_YYEOF;

    while (true)
    {
        LexerReturnType result = ScannerAutomaton::TKN_INTERNAL_SKIP;
        const char curr_ch = get_curr_char();
        switch (curr_ch)
        { // clang-format off
        case '=':  result = handle_equal_char();               break;
        case '!':  result = handle_exclamation_char();         break;
        case '+':  result = handle_plus_char();                break;
        case '-':  result = handle_minus_char();               break;
        case '<':  result = handle_left_angle_bracket_char();  break;
        case '>':  result = handle_right_angle_bracket_char(); break;
        case '.':  result = handle_dot_char();                 break;
        case ':':  result = handle_colon_char();               break;
        case '*':  result = TKN_MUL;                           break;
        case '/':  result = handle_slash_char();               break;
        case '%':  result = TKN_MOD;                           break;
        case '{':  result = TKN_LEFT_BRACE;                    break;
        case '}':  result = TKN_RIGHT_BRACE;                   break;
        case '[':  result = TKN_LEFT_BRACKET;                  break;
        case ']':  result = TKN_RIGHT_BRACKET;                 break;
        case '(':  result = TKN_LEFT_PAREN;                    break;
        case ')':  result = TKN_RIGHT_PAREN;                   break;
        case ';':  result = TKN_SEMICOLON;                     break;
        case ',':  result = TKN_COMMA;                         break;
        case '\"': result = handle_double_quote_char();        break;
        case '\n': register_newline_char();                    break;
        CASE_LIST_FOR_SPACES:                                  break;
        CASE_LIST_FOR_NUMBERS: result = handle_number_char();  break;
        CASE_LIST_FOR_LETTERS: result = handle_alpha_char();   break;
        default:
            {
                const char chartext[] = {curr_ch, 0};
                dr_.report_invalid_character(chartext, k_no_loc); break;
            }
        } // clang-format on

        advance_cursor(); // Main dispatching function always advance final token character.
        if (result != ScannerAutomaton::TKN_INTERNAL_SKIP)
            return result;
    }
}
#undef CASE_LIST_FOR_SPACES
#undef CASE_LIST_FOR_NUMBERS
#undef CASE_LIST_FOR_LETTERS
} // namespace alpha
