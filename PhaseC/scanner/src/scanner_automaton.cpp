#ifdef USE_FLEX_SCANNER
#error "ScannerAutomaton conflict: USE_FLEX_SCANNER is defined."
#endif

#include <alpha_parser.gen.hpp>
#include <charconv>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <scanner/scanner_automaton.hpp>

#include "../../core/include/core/escape_code_list.hpp"
#include "core/source_location_tracker.hpp"
#include "core/translation_unit_buffer.hpp"
#include "scanner/scanner_context.hpp"
#include "support/misc_tools.hpp"
#include "support/string_tools.hpp"

namespace
{
constexpr auto is_id_body_char = [](const unsigned char c) consteval
{
    return alpha::support::is_digit(c) || alpha::support::is_alpha(c) || c == '_';
};

constexpr auto g_id_body_table = []() consteval
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

static_assert(
    assert_id_body_table_integrity<0>(),
    "ID Table mismatch! See compiler output for index."
);
} // namespace

namespace alpha
{
ScannerAutomaton::ScannerAutomaton(
    LexerCtx& lexer_ctx,
    LocationTracker& lt,
    DiagnosticReporter& dr,
    const TranslationUnitBuffer& tub)
    : lexer_ctx_(lexer_ctx),
      lt_(lt),
      dr_(dr),
      tub_(tub),
      last_token_begin_(tub.data()),
      cursor_(tub.data())
{
    if (tub_.null_padding.value < ScannerAutomaton::k_minimum_source_buffer_null_padding)
        throw std::logic_error("Insufficient padding detected");
    for (auto pad_index = SrcBuffIdx{0}; pad_index < tub_.null_padding; ++pad_index)
        if (tub_[SrcBuffIdx{tub_.source_size() + pad_index}] != '\0')
            throw std::logic_error("Critical sentinel corruption detected");

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

template <SrcBuffIdx n>
char
ScannerAutomaton::get_nth_char() const noexcept
{
    // Guard the multi-character lookahead contract
    static_assert(ScannerAutomaton::k_minimum_source_buffer_null_padding >= n.value);
    DMASSERT(tub_.null_padding >= n);

    const char* const result_addr = cursor_ + n.value;
    DMASSERT(tub_.is_in_buffer(result_addr));
    return *result_addr;
}

char
ScannerAutomaton::get_nth_char(const SrcBuffIdx n) const noexcept
{
    const char* const result_addr = cursor_ + n.value;
    DMASSERT(tub_.is_in_buffer(result_addr));
    return *result_addr;
}

template <SrcBuffIdx n>
const char*
ScannerAutomaton::advance_cursor() noexcept
{
    static_assert(n.value > 0, "Why advance by zero?");
    DMASSERT(cursor_ < tub_.source_end()); // Is OK before?
    const auto result = cursor_ += n.value;
    // After: Can be AT source_size_ (EOF), but not past it.
    DMASSERT(cursor_ <= tub_.source_end()); // Is OK after?
    return result;
}

const char*
ScannerAutomaton::advance_cursor(const SrcBuffIdx n) noexcept
{
    DMASSERT(tub_.is_in_source(cursor_), n.value > 0 && "why advance 0?"); // OK before?
    const auto result = cursor_ += n.value;
    // After: Can be AT source_size_ (EOF), but not past it.
    DMASSERT(tub_.is_in_buffer(cursor_)); // OK after?
    return result;
}

char
ScannerAutomaton::get_curr_char() const noexcept
{
    DMASSERT(!has_reached_eof());
    return get_nth_char<SrcBuffIdx{0}>();
}

char
ScannerAutomaton::get_next_char() const noexcept
{
    DMASSERT(!has_reached_eof());
    return get_nth_char<SrcBuffIdx{1}>();
}

bool
ScannerAutomaton::has_reached_eof() const noexcept { return cursor_ == tub_.source_end(); }

SourceLocation
ScannerAutomaton::last_token_location() const noexcept
{
    return SourceLocation{
        SrcBuffIdx{static_cast<SrcBuffIdx::UnderlyingType>(last_token_begin() - tub_.begin())},
        SrcBuffIdx{static_cast<SrcBuffIdx::UnderlyingType>(last_token_end() - tub_.begin())},
    };
}

SourceLocation
ScannerAutomaton::last_token_location_eof_trimmed() const noexcept
{
    DMASSERT(has_reached_eof() && "Should only be called when eof is reached");
    SourceLocation loc = last_token_location();
    DMASSERT(
        loc.end > SrcBuffIdx{1} &&
        "Invalid EOF-trim: SourceLocation is empty or too small. "
        "`end` is exclusive and must reference a real character before trimming."
    );
    --loc.end;
    return loc;

    // EOF trimming is only used for unterminated tokens (strings or block comments).
    // When EOF is reached the scanner cursor has already advanced past the last
    // valid character, so `last_token_end()` points one past the real token end.
    // We therefore subtract one to obtain the actual source range.
    //
    // This assumes the token contains at least one real character. If `loc.end`
    // is 0 or 1, trimming would produce either an invalid range or `SourceLocation::none()`
    // (begin=0,end=0). That situation would imply we attempted to report an
    // unterminated token in an empty buffer or with a zero-length token, which
    // should be impossible under normal scanner invariants.
}

template <ScannerAutomaton::OpenerLen opener_len>
SourceLocation
ScannerAutomaton::calculate_opener_loc() const noexcept
{
    const auto begin =
        SrcBuffIdx{static_cast<SrcBuffIdx::UnderlyingType>(last_token_begin() - tub_.begin())};
    const auto end =
        SrcBuffIdx{begin.value + static_cast<std::underlying_type_t<OpenerLen>>(opener_len)};
    return SourceLocation{begin, end};
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::register_and_return(
    const LexerReturnType token_id,
    const SourceLocation token_loc) noexcept
{
    lexer_ctx_.register_token(TokenInfo{
        .id = static_cast<alpha_yytoken_kind_t>(token_id),
        .loc = token_loc
    });
    return token_id;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::register_and_return(const LexerReturnType token_id) noexcept
{
    return register_and_return(token_id, last_token_location());
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_equal_char() noexcept
{
    DMASSERT(get_curr_char() == '=');
    if (*advance_cursor() == '=')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_EQ);
    }
    return register_and_return(TKN_ASSIGN);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_exclamation_char() noexcept
{
    DMASSERT(get_curr_char() == '!');
    if (*advance_cursor() == '=') // Only place we support '!', is a part of != token.
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_NEQ);
    }
    dr_.report_invalid_character("!", last_token_location());
    return ScannerAutomaton::TKN_INTERNAL_SKIP;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_plus_char() noexcept
{
    DMASSERT(get_curr_char() == '+');
    if (*advance_cursor() == '+')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_INC);
    }
    return register_and_return(TKN_PLUS);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_minus_char() noexcept
{
    DMASSERT(get_curr_char() == '-');
    if (*advance_cursor() == '-')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_DEC);
    }
    return register_and_return(TKN_MINUS);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_left_angle_bracket_char() noexcept
{
    DMASSERT(get_curr_char() == '<');
    if (*advance_cursor() == '=')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_LTE);
    }
    return register_and_return(TKN_LT);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_right_angle_bracket_char() noexcept
{
    DMASSERT(get_curr_char() == '>');
    if (*advance_cursor() == '=')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_GTE);
    }
    return register_and_return(TKN_GT);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_dot_char() noexcept
{
    DMASSERT(get_curr_char() == '.');
    const char next_ch = get_next_char();
    if (support::is_digit(next_ch))
        return handle_float_number();

    advance_cursor();
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
    DMASSERT(get_curr_char() == ':');
    if (*advance_cursor() == ':')
    {
        advance_cursor(); // We need to advance cursor, as we just peeked (cursor wasn't moved).
        return register_and_return(TKN_GLOBAL);
    }
    return register_and_return(TKN_COLON);
}

void
ScannerAutomaton::register_newline_char() noexcept
{
    lt_.append_line(SrcBuffIdx{static_cast<SrcBuffIdx::UnderlyingType>(cursor_ - tub_.begin())});
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_comment_line() noexcept
{
    DMASSERT(get_curr_char() == '/', get_next_char() == '/');
    char curr_ch = *advance_cursor<SrcBuffIdx{2}>(); // for consume 1st and 2nd `/` (fast-path)
    while (!has_reached_eof() && curr_ch != '\n')
        curr_ch = *advance_cursor();
    return ScannerAutomaton::TKN_INTERNAL_SKIP;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_comment_block_nested() noexcept
{
    DMASSERT(get_curr_char() == '/', get_next_char() == '*');

    // Code could be more nicely written, but I specifically choose these patterns, to achieve max scanning speed. (micro opts)
    u64 block_comment_depth = 1;
    char ch1 = *advance_cursor<SrcBuffIdx{2}>();

    while (!has_reached_eof())
    {
        if (ch1 == '\n')
        {
            ch1 = *advance_cursor();
            register_newline_char();
            continue;
        }

        // Reminder even if ch2 is after last valid CHAR (so its after EOF), its still valid due to padding requirement (in ctor)
        const char ch2 = get_next_char();
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

        ch1 = *advance_cursor();
        if (block_comment_depth == 0)
            return ScannerAutomaton::TKN_INTERNAL_SKIP;
    }
    dr_.report_unclosed_comment(last_token_location_eof_trimmed());
    return TKN_YYEOF;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_comment_block_standard() noexcept
{
    DMASSERT(get_curr_char() == '/', get_next_char() == '*');
    char ch = *advance_cursor<SrcBuffIdx{2}>();
    while (!has_reached_eof())
    {
        if (ch == '\n')
        {
            ch = *advance_cursor();
            register_newline_char();
            continue;
        }

        // Reminder even if get_next_char() is after last valid CHAR (so its after EOF), its still valid due to padding requirement (in ctor)
        if (ch == '*' && get_next_char() == '/')
        {
            advance_cursor<SrcBuffIdx{2}>(); // consume '*' and '/' now,
            return ScannerAutomaton::TKN_INTERNAL_SKIP;
        }
        ch = *advance_cursor();
    }
    dr_.report_unclosed_comment(last_token_location_eof_trimmed());
    return TKN_YYEOF;
}


ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_slash_char() noexcept
{
    DMASSERT(get_curr_char() == '/');
    const char next_ch = get_next_char();
    if (next_ch == '/')
        return handle_comment_line();
    if (next_ch == '*')
        return handle_comment_block_standard();
    advance_cursor();
    return register_and_return(TKN_DIV);
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_double_quote_char() noexcept
{
    DMASSERT(get_curr_char() == '\"');
    // Just started handling string, so we first things first consume the initialization marker.
    advance_cursor();
    while (!has_reached_eof())
    {
        // Reminder even if ch2 is after last valid CHAR (so its after EOF), its still valid due to padding requirement (in ctor)
        const char ch1 = get_curr_char();
        advance_cursor();      // Consume ch1 here.
        if (ch1 == '\"')       // Handle matching "
            return TKN_STRING; // Let Main dispatcher consume the matching " char.
        if (ch1 == '\n')
            register_newline_char();
        else if (ch1 == '\\') // Handle potential escape code
        {
            const char ch2 = get_curr_char();
            advance_cursor(); // Consume ch2.
            switch (ch2)
            {
            #define EXTRACT_ESCAPE_CHARS(ch, escape_) case ch :
            ESCAPE_CODE_LIST(EXTRACT_ESCAPE_CHARS)
            #undef EXTRACT_ESCAPE_CHARS
                DMASSERT(!has_reached_eof() && "If ch2 past EOF, ch2 must be NULL-byte");
                break;
            default:
                const char chartext[] = {ch2, '\0'};
                dr_.report_invalid_escape_code(chartext, last_token_location());
                break;
            }
        }
    }

    dr_.report_unclosed_string(last_token_location_eof_trimmed());
    return TKN_YYEOF;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_hex_number() noexcept
{
    DMASSERT(
        get_nth_char<SrcBuffIdx{0}>() == '0',
        get_nth_char<SrcBuffIdx{1}>() == 'x' || get_nth_char<SrcBuffIdx{1}>() == 'X',
        support::is_xdigit(get_nth_char<SrcBuffIdx{2}>())
    );
    // We consume 0 and 'x' and the first digit we know it exists.
    char curr_ch = *advance_cursor<SrcBuffIdx{3}>();
    while (support::is_xdigit(curr_ch))
        curr_ch = *advance_cursor();
    DMASSERT(!support::is_xdigit(get_curr_char()));
    return TKN_INT;
}

// This function is a bit fat due to all the extra assertions.
// But I prefer it this way, although it could be leaned down to 20 lines.
ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_float_number() noexcept
{
    DMASSERT(get_curr_char() == '.', support::is_digit(get_next_char()));
    // --- Float digits consumption loop(in case of scientific, are all otherwise) --- //
    char curr_ch = '\0'; // Initializing to a sentinel, zero cost (Dead Store Elimination)
    while (support::is_digit(curr_ch = *advance_cursor())) // On first iteration consumes '.'
        continue;

    if (curr_ch == 'e' || curr_ch == 'E') // Handle Scientific floats
    {
        const auto consume_scientific_digits = [this]<SrcBuffIdx::UnderlyingType pre_advance>()
        {
            char ch = *advance_cursor<SrcBuffIdx{pre_advance}>();
            while (support::is_digit(ch)) ch = *advance_cursor();
        };

        const char next_ch = get_next_char(); // Valid cause 'curr_ch' was non null-byte.
        if ((next_ch == '+' || next_ch == '-') && support::is_digit(get_nth_char<SrcBuffIdx{2}>()))
            consume_scientific_digits.operator()<3>();
        else if (support::is_digit(next_ch))
            consume_scientific_digits.operator()<2>();
    }
    DMASSERT(!support::is_digit(get_curr_char()));
    return TKN_FLOAT;
}


ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_decimal_number() noexcept
{
    DMASSERT(support::is_digit(get_curr_char()), !has_reached_eof());

    char curr_ch = '\0'; // Initializing to a sentinel, zero cost (Dead Store Elimination)
    while (support::is_digit(curr_ch = *advance_cursor()))
        continue;
    if (curr_ch == '.' && support::is_digit(get_next_char()))
        return handle_float_number();
    return TKN_INT;
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_number_char() noexcept
{
    const char curr_ch = get_curr_char();
    DMASSERT(support::is_digit(curr_ch));

    // For HEX nums we require at least a hex digit after 0x as without it, 0x is just a decimal 0 and an id x.
    if (curr_ch == '0')
    {
        const char next_ch = get_next_char();
        if ((next_ch == 'x' || next_ch == 'X') && support::is_xdigit(get_nth_char<SrcBuffIdx{2}>()))
            return handle_hex_number();
    }

    // If here then the number isn't HEX, So we need to check if integer or float.
    return handle_decimal_number(); // This functions also handle floats starting with decimals.

    // Note float without decimal (ex: `.15`) are not detected here (but most likely on '.' handler).
}

ScannerAutomaton::LexerReturnType
ScannerAutomaton::handle_alpha_char() noexcept
{
    DMASSERT(support::is_alpha(get_curr_char()));
    SrcBuffIdx word_length{1};
    while (g_id_body_table[get_nth_char(word_length)])
        ++word_length;

    DMASSERT(g_id_body_table[get_curr_char()]);

    const auto find_possible_keyword = [this, word_length]() -> KeywordId
    {
        const char ch0 = get_curr_char();
        switch (word_length.value)
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
                const char ch1 = get_next_char();
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
        if (std::string_view{cursor_, word_length.value} == expected_token.name)
            result_token = expected_token.bison_token_id;
    }

    advance_cursor(SrcBuffIdx{word_length.value});
    DMASSERT(has_reached_eof() || !g_id_body_table[get_curr_char()]);
    return result_token;
}

void
ScannerAutomaton::handle_newline_char() noexcept
{
    advance_cursor();
    register_newline_char();
}


void
ScannerAutomaton::handle_invalid_char(const char curr_ch) noexcept
{
    DMASSERT(get_curr_char() == curr_ch);
    advance_cursor();
    const char chartext[] = {curr_ch, '\0'};
    dr_.report_invalid_character(chartext, last_token_location());
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
ScannerAutomaton::yield_token(YYSTYPE* const yylval, YYLTYPE* const yylloc) noexcept
{
    const auto consume_token = [this]<LexerReturnType token>()
    {
        advance_cursor();
        return token;
    };

    // DMASSERT(tub_.is_in_source(cursor_));

    while (true)
    {
        last_token_begin_ = cursor_;
        if (!tub_.is_in_source(cursor_))
            return register_and_return(TKN_YYEOF, *yylloc = SourceLocation::eof());

        LexerReturnType result = ScannerAutomaton::TKN_INTERNAL_SKIP;
        const char curr_ch = get_curr_char();
        switch (curr_ch)
        { // clang-format off
        case '=':  result = handle_equal_char();                           break;
        case '!':  result = handle_exclamation_char();                     break;
        case '+':  result = handle_plus_char();                            break;
        case '-':  result = handle_minus_char();                           break;
        case '<':  result = handle_left_angle_bracket_char();              break;
        case '>':  result = handle_right_angle_bracket_char();             break;
        case '.':  result = handle_dot_char();                             break;
        case ':':  result = handle_colon_char();                           break;
        case '*':  result = consume_token.operator()<TKN_MUL>();           break;
        case '/':  result = handle_slash_char();                           break;
        case '%':  result = consume_token.operator()<TKN_MOD>();           break;
        case '{':  result = consume_token.operator()<TKN_LEFT_BRACE>();    break;
        case '}':  result = consume_token.operator()<TKN_RIGHT_BRACE>();   break;
        case '[':  result = consume_token.operator()<TKN_LEFT_BRACKET>();  break;
        case ']':  result = consume_token.operator()<TKN_RIGHT_BRACKET>(); break;
        case '(':  result = consume_token.operator()<TKN_LEFT_PAREN>();    break;
        case ')':  result = consume_token.operator()<TKN_RIGHT_PAREN>();   break;
        case ';':  result = consume_token.operator()<TKN_SEMICOLON>();     break;
        case ',':  result = consume_token.operator()<TKN_COMMA>();         break;
        case '\"': result = handle_double_quote_char();                    break;
        case '\n': handle_newline_char();                                  break;
        CASE_LIST_FOR_SPACES:  advance_cursor();                           break;
        CASE_LIST_FOR_NUMBERS: result = handle_number_char();              break;
        CASE_LIST_FOR_LETTERS: result = handle_alpha_char();               break;
        default: handle_invalid_char(curr_ch);                             break;
        } // clang-format on

        switch (result)
        {
        case TKN_INT:
            std::from_chars(last_token_begin_, cursor_, yylval->const_int);
            break;
        case TKN_FLOAT:
            std::from_chars(last_token_begin_, cursor_, yylval->const_float);
            break;
        case TKN_ID:
        case TKN_STRING:
            {
                const std::string_view text = last_token_text();
                yylval->string = StringSpan{.data = text.data(), .size = text.size()};
                break;
            }
        case TKN_YYEOF:
            return register_and_return(TKN_YYEOF, *yylloc = SourceLocation::eof());
        case ScannerAutomaton::TKN_INTERNAL_SKIP:
            continue;
        }
        return register_and_return(result, *yylloc = last_token_location());
    }
}
#undef CASE_LIST_FOR_SPACES
#undef CASE_LIST_FOR_NUMBERS
#undef CASE_LIST_FOR_LETTERS
} // namespace alpha
