/// PLEASE DO NOT LOOK AT THIS FILE IS A MESS (PARTIALLY DUE TO BISON API)
/// BUT BASICALLY ITS MY FAULT... WHEN WRITING THIS.. I WAS BORED AND ANXIOUS...
/// SO PLEASE DONT JUDGE ME TOO HARSH...

#ifndef PARSER_EPILOGUE_CODE_HPP
#define PARSER_EPILOGUE_CODE_HPP
// TODO: When you have the time.. please fix-up / rewrite this file.. its a mess
// although i feel like it has potential.
#include <alpha_parser.gen.hpp>
#include <limits>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <diagnostics/diagnostic_reporter.gen.hpp>

#include "core/source_location_tracker.hpp"
#include "diagnostics/diagnostic_types.hpp"
#include "L1_driver/semantic_system.hpp"
#include "scanner/scanner_context.hpp"
#include "support/debug_tools.hpp"

using namespace alpha;
constexpr Issue::Type SYNTAX_ERROR_ISSUE_TYPE = Issue::Type::HARD_ERROR;
constexpr unsigned FEW_TOKENS = 4;
constexpr int YYREPORT_SYNTAX_ERROR_RETVAL = 0;

struct Info
{
    const yysymbol_kind_t unexpected_token;
    const char* const unexpected_token_name;
    const std::string_view unexpected_token_source_text;
    const YYLTYPE unexpected_token_loc;
    std::vector<yysymbol_kind_t> expected_tokens;
};

[[nodiscard]] static bool
has_expected(const Info& info, const yysymbol_kind_t s)
{
    for (std::size_t i = 0; i < info.expected_tokens.size(); ++i)
        if (info.expected_tokens[i] == s)
            return true;
    return false;
};

[[nodiscard]] static yysymbol_kind_t
determine_suggested_token_based_on_parsing_heuristics(
    const SemanticSystem& ss,
    const LocationTracker& loc_tracker,
    const LexerCtx& lexer_ctx,
    const Info& info)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY &&
        "Should not be called without unexpected symbol"
    );

    if (!lexer_ctx.second_last_token_info().has_value())
        return YYSYMBOL_YYEMPTY;

    const TokenInfo slast_token_info = lexer_ctx.second_last_token_info().value();
    const auto slast_last_line = loc_tracker.find_last_line(slast_token_info.loc);
    const TokenInfo last_token_info = lexer_ctx.last_token_info().value();
    const auto unexpected_first_line = loc_tracker.find_first_line(info.unexpected_token_loc);
    // Suggestion priority:   `;`  `:`  `)`  `]`  `}`  `,`
    // NOTE: because there are likely too many option (this function is used in too_many_expected)
    // we only suggest semicolon if a newline is in between the second last token and last token..
    // (the unexpected one), that is a heuristic that usually wins.. Suggesting semicolon without
    // this rule, make suggestion awful as semicolon can be placed in many places that most of time
    // make no sense.
    if (has_expected(info, YYSYMBOL_SEMICOLON) &&
        last_token_info.id != alpha_yytoken_kind_t::TKN_SEMICOLON &&
        last_token_info.id != alpha_yytoken_kind_t::TKN_RIGHT_BRACE &&
        slast_token_info.id != alpha_yytoken_kind_t::TKN_SEMICOLON &&
        slast_token_info.id != alpha_yytoken_kind_t::TKN_RIGHT_BRACE &&
        slast_last_line < unexpected_first_line) { return YYSYMBOL_SEMICOLON; }
    if (has_expected(info, YYSYMBOL_COLON)) return YYSYMBOL_COLON;
    if (has_expected(info, YYSYMBOL_COMMA) &&
        (
            ss.context_inspector->is_in_func_param_list() ||
            ss.context_inspector->is_in_call_arg_list()
        )) { return YYSYMBOL_COMMA; }
    if (has_expected(info, YYSYMBOL_RIGHT_PAREN)) return YYSYMBOL_RIGHT_PAREN;
    if (has_expected(info, YYSYMBOL_RIGHT_BRACKET)) return YYSYMBOL_RIGHT_BRACKET;
    if (has_expected(info, YYSYMBOL_RIGHT_BRACE)) return YYSYMBOL_RIGHT_BRACE;
    if (has_expected(info, YYSYMBOL_COMMA)) return YYSYMBOL_COMMA;

    // if nothing else matches .. we revisit semicolon
    if (has_expected(info, YYSYMBOL_SEMICOLON) &&
        slast_token_info.id != alpha_yytoken_kind_t::TKN_SEMICOLON &&
        slast_token_info.id != alpha_yytoken_kind_t::TKN_RIGHT_BRACE) { return YYSYMBOL_SEMICOLON; }
    return YYSYMBOL_YYEMPTY;
}

[[nodiscard]] bool
made_diagnostic_based_on_semantic_heuristics(
    const SemanticSystem& ss,
    const LexerCtx& lexer_ctx,
    DiagnosticReporter& dr,
    const Info& info)
{
    DEBUG_SMART_ASSERT(info.unexpected_token != YYSYMBOL_YYEMPTY &&
        "Should not be called without unexpected symbol");

    if (!lexer_ctx.second_last_token_info().has_value())
        return false;
    if (info.expected_tokens.empty())
        return false;

    if (ss.context_inspector->is_in_func_param_list() &&
        info.unexpected_token == YYSYMBOL_LEFT_BRACE &&
        has_expected(info, YYSYMBOL_RIGHT_PAREN))
    {
        const auto expected = yysymbol_name(YYSYMBOL_RIGHT_PAREN);

        dr.report_syntax_error_expected_before(
            expected,
            info.unexpected_token_name,
            info.unexpected_token_loc,
            lexer_ctx.second_last_token_info()->loc
        );
        return true;
    }
    if (ss.context_inspector->is_in_func_param_list() &&
        has_expected(info, YYSYMBOL_COMMA))
    {
        auto expected = yysymbol_name(YYSYMBOL_COMMA);
        std::string error_message;
        if (info.unexpected_token == YYSYMBOL_ID)
            dr.report_syntax_error_expected(
                expected,
                info.unexpected_token_loc,
                lexer_ctx.second_last_token_info()->loc
            );
        else
            dr.report_syntax_error_expected_instead_of(
                expected,
                info.unexpected_token_name,
                expected,
                info.unexpected_token_loc,
                lexer_ctx.second_last_token_info()->loc
            );
        return true;
    }
    if (ss.context_inspector->is_in_func_param_list() &&
        info.unexpected_token == YYSYMBOL_RIGHT_PAREN &&
        lexer_ctx.second_last_token_info().has_value() &&
        lexer_ctx.second_last_token_info().value().id == TKN_COMMA)
    {
        dr.report_syntax_error_remove_or_add(
            yysymbol_name(YYSYMBOL_COMMA),
            yysymbol_name(YYSYMBOL_ID),
            lexer_ctx.second_last_token_info()->loc
        );
        return true;
    }
    return false;
}

// Expression Heuristic, basically an educated guess, that we expected upcoming expression.
bool expected_primary_expression(const Info& info)
{
    // All the symbols below can be a starter of expr... the more symbol we check are valid
    // the stronger we make our heuristic.
    return has_expected(info, YYSYMBOL_ID) &&
           has_expected(info, YYSYMBOL_NIL) &&
           has_expected(info, YYSYMBOL_TRUE) &&
           has_expected(info, YYSYMBOL_FALSE) &&
           has_expected(info, YYSYMBOL_INT) &&
           has_expected(info, YYSYMBOL_FLOAT) &&
           has_expected(info, YYSYMBOL_STRING) &&
           has_expected(info, YYSYMBOL_LEFT_PAREN) &&
           has_expected(info, YYSYMBOL_NOT) &&
           has_expected(info, YYSYMBOL_MINUS) &&
           has_expected(info, YYSYMBOL_DEC) &&
           has_expected(info, YYSYMBOL_INC) &&
           has_expected(info, YYSYMBOL_LOCAL) &&
           has_expected(info, YYSYMBOL_GLOBAL);
}

[[nodiscard]] static unsigned int
yypcontext_expected_tokens_or_throw(
    const yypcontext_t* const yyctx,
    yysymbol_kind_t yyarg[],
    const unsigned int yyargn)
{
    DEBUG_SMART_ASSERT(!!yyctx, !!yyarg || !yyargn);

    // We assert this, as original yypcontext_expected_tokens(), receives int for yyargn.
    DEBUG_SMART_ASSERT(yyargn <= std::numeric_limits<int>::max());

    int expected_count = yypcontext_expected_tokens(yyctx, yyarg, yyargn);
    if (expected_count >= 0)
        return static_cast<unsigned int>(expected_count);

    throw std::runtime_error(ATTACH_CONTEXT(FMT::format(
        "Bison: yypcontext_expected_tokens() returned a negative value\n"
        "(internal failure, commonly due to memory exhaustion).\n"
        "Verify call contract: (yyarg == nullptr) == (yyargn == 0),\n"
        "and ensure the output buffer is sufficiently large.\n"
        "Buffer size of yyarg = {0}\n",
        yyargn
    )));
}

[[nodiscard]] static unsigned int
expected_size(const yypcontext_t* const yyctx)
{
    DEBUG_SMART_ASSERT(!!yyctx);
    return yypcontext_expected_tokens_or_throw(yyctx, nullptr, 0);
}

[[nodiscard]] static std::vector<yysymbol_kind_t>
collect_expected_tokens(const yypcontext_t* const yyctx)
{
    unsigned int count = expected_size(yyctx);
    if (count == 0)
        return {};

    std::vector<yysymbol_kind_t> result(count);

    [[maybe_unused]] unsigned int filled = // Only used in DEBUG
        yypcontext_expected_tokens_or_throw(yyctx, result.data(), count);
    DEBUG_SMART_ASSERT(filled == count && filled == result.size());
    return result;
}

[[nodiscard]] static std::string
get_formatted_unexpected_token_name(const Info& info)
{
    DEBUG_SMART_ASSERT(!!info.unexpected_token_name, !!info.unexpected_token_source_text.data());
    std::string out(info.unexpected_token_name);
    switch (info.unexpected_token)
    {
    case YYSYMBOL_ID:
    case YYSYMBOL_INT:
    case YYSYMBOL_FLOAT:
        out += ' ';
        out += info.unexpected_token_source_text;
        break;
    case YYSYMBOL_STRING:
        out += ' ';
        out += '\"';
        out += info.unexpected_token_source_text;
        out += '\"';
        break;
    default: ((void)0);
    }
    return out;
}

[[maybe_unused]][[nodiscard]] static Suggestion
make_symbol_suggestion(const LexerCtx& lexer_ctx, yysymbol_kind_t suggested_symbol)
{
    DEBUG_SMART_ASSERT(suggested_symbol != YYSYMBOL_YYEMPTY);
    const TokenInfo token_info = lexer_ctx.last_token_info().value();
    return Suggestion{yysymbol_name(suggested_symbol), token_info.loc};
}

static void
report_no_expected_diagnostic(const Info& info, DiagnosticReporter& dr)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.empty());

    dr.report_syntax_error_unexpected(
        get_formatted_unexpected_token_name(info),
        info.unexpected_token_loc
    );
}

#include "diagnostics/issue_formatter.hpp"

static void
report_few_expected_diagnostic(
    const LexerCtx& lexer_ctx,
    const Info& info,
    DiagnosticReporter& dr
)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.size() <= FEW_TOKENS);
    const auto join_expected = [&](const char* const sep, const bool wrap_with_decorator)
    {
        std::string out;
        for (std::size_t i = 0; i < info.expected_tokens.size(); i++)
        {
            out += i != 0 ? sep : "";
            out += wrap_with_decorator ? "‘" : "";
            out += yysymbol_name(info.expected_tokens[i]);
            out += wrap_with_decorator ? "‘" : "";
        }
        return out;
    };

    const auto last_token_info = lexer_ctx.last_token_info();
    const auto second_last_token_info = lexer_ctx.second_last_token_info();

    if (last_token_info.has_value() && second_last_token_info)
        dr.report_syntax_error_expected_instead_of(
            join_expected(" or ", true),
            get_formatted_unexpected_token_name(info),
            join_expected("\n", false),
            info.unexpected_token_loc,
            last_token_info->loc == info.unexpected_token_loc
            ? second_last_token_info->loc
            : last_token_info->loc
        );
    else
        dr.report_syntax_error_expected_instead_of_without_suggestion(
            join_expected(" or ", true),
            get_formatted_unexpected_token_name(info),
            info.unexpected_token_loc
        );
}

static void report_unexpected_eof(
    const LexerCtx& lexer_ctx,
    DiagnosticReporter& dr,
    const Info& info)
{
    DEBUG_SMART_ASSERT(info.unexpected_token == YYSYMBOL_YYEOF);
    const std::optional<TokenInfo> last_token_info_opt = lexer_ctx.last_token_info();
    std::optional<Suggestion> suggestion;

    const auto report_generic_eof = [&dr, &info]()
    {
        dr.report_syntax_error_unexpected(info.unexpected_token_name, info.unexpected_token_loc);
    };

    if (!last_token_info_opt.has_value())
    {
        report_generic_eof();
        return;
    }

    yysymbol_kind_t expected = YYSYMBOL_YYEMPTY;
    // Suggestion priority:   `;`  `:`  `)`  `]`  `}`  `,`
    if (has_expected(info, YYSYMBOL_SEMICOLON) &&
        last_token_info_opt.has_value() &&
        last_token_info_opt->id != alpha_yytoken_kind_t::TKN_SEMICOLON &&
        last_token_info_opt->id != alpha_yytoken_kind_t::TKN_RIGHT_BRACE)
    {
        expected = YYSYMBOL_SEMICOLON;
    }
    if (has_expected(info, YYSYMBOL_COLON)) expected = YYSYMBOL_COLON;
    if (has_expected(info, YYSYMBOL_RIGHT_PAREN)) expected = YYSYMBOL_RIGHT_PAREN;
    if (has_expected(info, YYSYMBOL_RIGHT_BRACKET)) expected = YYSYMBOL_RIGHT_BRACKET;
    if (has_expected(info, YYSYMBOL_RIGHT_BRACE)) expected = YYSYMBOL_RIGHT_BRACE;
    if (has_expected(info, YYSYMBOL_COMMA)) expected = YYSYMBOL_COMMA;

    const char* opener_name;
    std::optional<SourceLocation> opener_loc;
    switch (expected)
    {
    case YYSYMBOL_RIGHT_PAREN:
        opener_loc = lexer_ctx.lastest_open_parenthesis_loc();
        opener_name = yysymbol_name(YYSYMBOL_LEFT_PAREN);
        break;
    case YYSYMBOL_RIGHT_BRACKET:
        opener_loc = lexer_ctx.lastest_open_bracket_loc();
        opener_name = yysymbol_name(YYSYMBOL_LEFT_BRACKET);
        break;
    case YYSYMBOL_RIGHT_BRACE:
        opener_loc = lexer_ctx.latest_open_brace_loc();
        opener_name = yysymbol_name(YYSYMBOL_LEFT_BRACE);
        break;
    default: break;
    }

    if (expected != YYSYMBOL_YYEMPTY)
    {
        const char* const expected_name = yysymbol_name(expected);
        std::optional<Suggestion> suggestion;
        if (last_token_info_opt.has_value())
            suggestion.emplace(expected_name, last_token_info_opt->loc);

        if (opener_loc.has_value())
        {
            dr.report_syntax_error_expected_closer(
                expected_name,
                get_formatted_unexpected_token_name(info),
                opener_name,
                info.unexpected_token_loc,
                lexer_ctx.last_token_info().value().loc,
                *opener_loc
            );
            return;
        }
        dr.report_syntax_error_expected_before(
            expected_name,
            get_formatted_unexpected_token_name(info),
            info.unexpected_token_loc,
            lexer_ctx.last_token_info().value().loc
        );
        return;
    }
    report_generic_eof();
}

static void
report_many_expected_diagnostic(
    const SemanticSystem& ss,
    const LocationTracker& loc_tracker,
    const LexerCtx& lexer_ctx,
    const Info& info,
    DiagnosticReporter& dr)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.size() > FEW_TOKENS);
    const auto unexpected_token_str = get_formatted_unexpected_token_name(info);
    std::optional<Suggestion> suggestion;
    std::list<Note> notes;

    const yysymbol_kind_t suggested_symbol =
        determine_suggested_token_based_on_parsing_heuristics(ss, loc_tracker, lexer_ctx, info);

    if (suggested_symbol == YYSYMBOL_YYEMPTY)
    {
        if (expected_primary_expression(info) && lexer_ctx.second_last_token_info().has_value())
        {
            dr.report_syntax_error_expected_expression(
                unexpected_token_str,
                info.unexpected_token_loc,
                lexer_ctx.second_last_token_info().value().loc
            );
            return;
        }

        if (info.unexpected_token == YYSYMBOL_YYEOF)
        {
            report_unexpected_eof(lexer_ctx, dr, info);
            return;
        }
        dr.report_syntax_error_unexpected(unexpected_token_str, info.unexpected_token_loc);
    }
    else
    {
        const char* opener_name = nullptr;
        std::optional<SourceLocation> opener_loc;
        switch (suggested_symbol)
        {
        case YYSYMBOL_RIGHT_PAREN:
            opener_loc = lexer_ctx.lastest_open_parenthesis_loc();
            opener_name = yysymbol_name(YYSYMBOL_LEFT_PAREN);
            break;
        case YYSYMBOL_RIGHT_BRACKET:
            opener_loc = lexer_ctx.lastest_open_bracket_loc();
            opener_name = yysymbol_name(YYSYMBOL_LEFT_BRACKET);
            break;
        case YYSYMBOL_RIGHT_BRACE:
            opener_loc = lexer_ctx.latest_open_brace_loc();
            opener_name = yysymbol_name(YYSYMBOL_LEFT_BRACE);
            break;
        default: break;
        }

        if (opener_loc.has_value() && lexer_ctx.last_token_info().has_value())
        {
            DEBUG_SMART_ASSERT(opener_loc.value() != SourceLocation::none());
            dr.report_syntax_error_expected_closer(
                yysymbol_name(suggested_symbol),
                unexpected_token_str,
                opener_name,
                info.unexpected_token_loc,
                lexer_ctx.last_token_info().value().loc,
                *opener_loc
            );
            return;
        }
    }
    if (lexer_ctx.second_last_token_info().has_value())
        dr.report_syntax_error_expected_before(
            yysymbol_name(suggested_symbol),
            unexpected_token_str,
            info.unexpected_token_loc,
            lexer_ctx.second_last_token_info().value().loc
        );
    else
        dr.report_syntax_error_unexpected(unexpected_token_str, info.unexpected_token_loc);
}

static void
report_no_unexpected_diagnostic(const YYLTYPE unexpected_loc, DiagnosticReporter& dr)
{
    DEBUG_SMART_ASSERT(false &&
        "HOLD YOUR HORSES... You just caused an error,\n"
        "you have no idea how to replicate. Somehow parse\n"
        "encountered an error without a token (an unexpected)\n"
        "Basically if there is no unexpected, it must mean\n"
        "that there is NOTHING to cause an error... Right?\n"
    );

    dr.report___wtf__(
        "Hey a syntax error occurred, PLEASE if you see this message, "
        "contact the developer and tell him you got this message."
        "Also if you are kind enough, provide him with the source-file "
        "that cause this error. Parser is deterministic,"
        "so he should be able to replicate. THANK YOU",
        unexpected_loc
    );
}

static void
report_unexpected_diagnostic(
    const LocationTracker& loc_tracker,
    const LexerCtx& lexer_ctx,
    const Info& info,
    DiagnosticReporter& dr,
    const SemanticSystem& ss)
{
    if (info.expected_tokens.empty())
        report_no_expected_diagnostic(info, dr);
    else if (made_diagnostic_based_on_semantic_heuristics(ss, lexer_ctx, dr, info))
        return;
    else if (info.expected_tokens.size() <= FEW_TOKENS)
        report_few_expected_diagnostic(lexer_ctx, info, dr);
    else
        report_many_expected_diagnostic(ss, loc_tracker, lexer_ctx, info, dr);
}

/**
 * @returns Always return 0.
 * @brief
 * Per the Bison manual, yyreport_syntax_error should return 0 or YYENOMEM
 * on allocation failure. Our driver handles allocation failures via C++
 * exceptions instead. Additionally, the generated parser we target (Bison
 * 3.8.2 on Fedora) checks a hard-coded literal `2` rather than `YYENOMEM`
 * when propagating memory exhaustion from yyreport_syntax_error. To avoid
 * coupling to that quirk and to keep behavior stable across versions, we
 * do not signal exhaustion via the return value. If an allocation fails in
 * this function, we throw; otherwise we emit diagnostics and return 0.
 */
static int yyreport_syntax_error(
    yypcontext_t const* yyctx,
    ScannerAdapter &scanner,
    LexerCtx& lexer_ctx,
    LocationTracker& loc_tracker,
    [[maybe_unused]] DiagnosticEngine& diagnostic_engine,
    DiagnosticReporter& dr,
    SemanticSystem& ss)
{
    const yysymbol_kind_t unexpected_token = yypcontext_token(yyctx);
    const YYLTYPE unexpected_token_loc = *support::require_ptr(yypcontext_location(yyctx));

    if (unexpected_token == YYSYMBOL_YYEMPTY)
    // According to bison manual this mean NO-LOOKAHEAD
        report_no_unexpected_diagnostic(unexpected_token_loc, dr);
    else
    {
        const auto info = Info{
            .unexpected_token = unexpected_token,
            .unexpected_token_name = yysymbol_name(unexpected_token),
            .unexpected_token_source_text = scanner.last_token_text(),
            .unexpected_token_loc = unexpected_token_loc,
            .expected_tokens = collect_expected_tokens(yyctx)
        };
        report_unexpected_diagnostic(loc_tracker, lexer_ctx, info, dr, ss);
    }
    return YYREPORT_SYNTAX_ERROR_RETVAL;
}

static void alpha_yyerror(
    const ALPHA_YYLTYPE* const err_loc,
    const alpha::ScannerAdapter&,
    const alpha::LexerCtx &,
    const alpha::LocationTracker&,
    const alpha::DiagnosticEngine&,
    alpha::DiagnosticReporter& dr,
    const alpha::SemanticSystem&,
    const std::string&)
{
    // TODO: static_assert(false, "WRITE ME NICELY!");
    dr.report_parser_stack_exhausted(PARSER_STACK_CAPACITY, *err_loc);
}

#endif // PARSER_EPILOGUE_CODE_HPP
