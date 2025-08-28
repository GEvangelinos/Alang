#ifndef PARSER_EPILOGUE_CODE_HPP
#define PARSER_EPILOGUE_CODE_HPP

#include <alpha_parser.gen.hpp>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <scanner/alpha_scanner.gen.hpp>

#include "diagnostics/diagnostic_types.hpp"
#include "L1_driver/semantic_system.hpp"
#include "scanner/scanner_context.hpp"
#include "utils/debug_tools.hpp"

using namespace alpha;
constexpr Issue::Type SYNTAX_ERROR_ISSUE_TYPE = Issue::Type::HARD_ERROR;
constexpr unsigned FEW_TOKENS = 4;
constexpr int YYREPORT_SYNTAX_ERROR_RETVAL = 0;

struct Info
{
    const yysymbol_kind_t unexpected_token;
    const char *const unexpected_token_name;
    const char *const unexpected_token_source_text;
    const YYLTYPE unexpected_token_loc;
    std::vector<yysymbol_kind_t> expected_tokens;
};

[[nodiscard]] static bool
has_expected(const Info &info, const yysymbol_kind_t s)
{
    for (std::size_t i = 0; i < info.expected_tokens.size(); ++i)
        if (info.expected_tokens[i] == s)
            return true;
    return false;
};

[[nodiscard]] static yysymbol_kind_t
determine_suggested_token(const LexerCtx &lexer_ctx, const Info &info)
{
    const TokenInfo token_info = lexer_ctx.second_last_token_info().value();

    // Suggestion priority:   `;`  `:`  `)`  `]`  `}`  `,`
    if (has_expected(info, YYSYMBOL_SEMICOLON) &&
        token_info.id != alpha_yytoken_kind_t::SEMICOLON &&
        token_info.id != alpha_yytoken_kind_t::RIGHT_BRACE) { return YYSYMBOL_SEMICOLON; }
    if (has_expected(info, YYSYMBOL_COLON)) return YYSYMBOL_COLON;
    if (has_expected(info, YYSYMBOL_RIGHT_PAREN)) return YYSYMBOL_RIGHT_PAREN;
    if (has_expected(info, YYSYMBOL_RIGHT_BRACKET)) return YYSYMBOL_RIGHT_BRACKET;
    if (has_expected(info, YYSYMBOL_RIGHT_BRACE)) return YYSYMBOL_RIGHT_BRACE;
    if (has_expected(info, YYSYMBOL_COMMA)) return YYSYMBOL_COMMA;
    return YYSYMBOL_YYEMPTY;
}

// Expression Heuristic, basically an educated guess, that we expected upcoming expression.
bool expected_primary_expression(const Info &info)
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
    const yypcontext_t *const yyctx,
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
expected_size(const yypcontext_t *const yyctx)
{
    DEBUG_SMART_ASSERT(!!yyctx);
    return yypcontext_expected_tokens_or_throw(yyctx, nullptr, 0);
}

[[nodiscard]] static std::vector<yysymbol_kind_t>
collect_expected_tokens(const yypcontext_t *const yyctx)
{
    unsigned int count = expected_size(yyctx);
    if (count == 0)
        return {};

    std::vector<yysymbol_kind_t> result(count);
    unsigned int filled = yypcontext_expected_tokens_or_throw(yyctx, result.data(), count);
    DEBUG_SMART_ASSERT(filled == count && filled == result.size());

    return result;
}

[[nodiscard]] static std::string
get_formatted_unexpected_token_name(const Info &info)
{
    DEBUG_SMART_ASSERT(!!info.unexpected_token_name, !! info.unexpected_token_source_text);
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
    default: ((void) 0);
    }
    return out;
}

[[nodiscard]] static Suggestion
make_symbol_suggestion(const LexerCtx &lexer_ctx, yysymbol_kind_t suggested_symbol)
{
    DEBUG_SMART_ASSERT(suggested_symbol != YYSYMBOL_YYEMPTY);
    const TokenInfo token_info = lexer_ctx.second_last_token_info().value();
    return Suggestion{yysymbol_name(suggested_symbol), token_info.loc};
}

static void
report_no_expected_diagnostic(const Info &info, DiagnosticEngine &diagnostic_engine)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.empty());

    return diagnostic_engine.report(Issue(
        SYNTAX_ERROR_ISSUE_TYPE,
        FMT::format("unexpected `{}`", get_formatted_unexpected_token_name(info)),
        info.unexpected_token_loc
    ));
}

static void
report_few_expected_diagnostic(
    const LexerCtx &lexer_ctx,
    const Info &info,
    DiagnosticEngine &diagnostic_engine
)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.size() <= FEW_TOKENS);
    auto join_expected = [&](const char *const sep, bool wrap_with_backsticks) -> std::string
    {
        std::string out;
        for (std::size_t i = 0; i < info.expected_tokens.size(); i++)
        {
            out += i != 0 ? sep : "";
            out += wrap_with_backsticks ? "`" : "";
            out += yysymbol_name(info.expected_tokens[i]);
            out += wrap_with_backsticks ? "`" : "";
        }
        return out;
    };

    std::optional<Suggestion> suggestion;
    if (const auto token_info = lexer_ctx.second_last_token_info(); token_info.has_value())
        suggestion.emplace(join_expected("\n", false), token_info->loc);

    diagnostic_engine.report(Issue(
        SYNTAX_ERROR_ISSUE_TYPE,
        FMT::format("expected {} before `{}`", join_expected(" or ", true),
                    get_formatted_unexpected_token_name(info)),
        info.unexpected_token_loc,
        suggestion
    ));
}

static void
report_too_many_expected_diagnostic(
    const LexerCtx &lexer_ctx,
    const Info &info,
    DiagnosticEngine &diagnostic_engine)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.size() > FEW_TOKENS);
    const auto unexpected_str = get_formatted_unexpected_token_name(info);
    std::optional<Suggestion> suggestion;
    std::list<Note> notes;
    if (expected_primary_expression(info) && lexer_ctx.second_last_token_info().has_value())
    {
        suggestion.emplace("expression", lexer_ctx.second_last_token_info().value().loc);
        Issue primary = Issue(
            SYNTAX_ERROR_ISSUE_TYPE,
            FMT::format("expected expression, found ‘{}’", unexpected_str),
            info.unexpected_token_loc,
            suggestion
        );
        diagnostic_engine.report(std::move(primary), std::move(notes));
        return;
    }

    const yysymbol_kind_t suggested_symbol = determine_suggested_token(lexer_ctx, info);

    if (suggested_symbol != YYSYMBOL_YYEMPTY)
    {
        suggestion.emplace(make_symbol_suggestion(lexer_ctx, suggested_symbol));

        const char *opener_name = nullptr;
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

        if (opener_loc.has_value())
        {
            DEBUG_SMART_ASSERT(opener_loc.value() != k_no_loc);
            notes.emplace_back(
                FMT::format("to match this `{}`", DEBUG_REQUIRE_PTR(opener_name)),
                opener_loc.value()
            );
        }
    }

    Issue primary = Issue(
        SYNTAX_ERROR_ISSUE_TYPE,
        FMT::format("invalid syntax, unexpected ‘{}’", unexpected_str),
        info.unexpected_token_loc,
        suggestion
    );

    diagnostic_engine.report(std::move(primary), std::move(notes));
}

static void
report_no_unexpected_diagnostic(const YYLTYPE unexpected_loc,
                                DiagnosticEngine &diagnostic_engine)
{
    DEBUG_SMART_ASSERT(false &&
        "HOLD YOUR HORSES... You just caused an error,\n"
        "you have no idea how to replicate. Somehow parse\n"
        "encountered an error without a token (an unexpected)\n"
        "Basically if there is no unexpected, it must mean\n"
        "that there is NOTHING to cause an error... Right?\n"
    );
    diagnostic_engine.report(Issue(
        Issue::Type::FATAL_ERROR,
        "Hey a syntax error occurred, PLEASE if you see this message, "
        "contact the developer and tell him you got this message."
        "Also if you are kind enough, provide him with the source-file "
        "that cause this error. Parser is deterministic,"
        "so he should be able to replicate. THANK YOU",
        unexpected_loc
    ));
}

static void
report_unexpected_diagnostic(
    const LexerCtx &lexer_ctx,
    const Info &info,
    DiagnosticEngine &diagnostic_engine)
{
    if (info.expected_tokens.empty())
        report_no_expected_diagnostic(info, diagnostic_engine);
    else if (info.expected_tokens.size() <= FEW_TOKENS)
        report_few_expected_diagnostic(lexer_ctx, info, diagnostic_engine);
    else
        report_too_many_expected_diagnostic(lexer_ctx, info, diagnostic_engine);
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
    yypcontext_t const *yyctx,
    const yyscan_t flex_ctx,
    LexerCtx &lexer_ctx,
    [[maybe_unused]] LocationTracker &lt,
    DiagnosticEngine &diagnostic_engine,
    [[maybe_unused]] DiagnosticReporter &dr,
    [[maybe_unused]] SemanticSystem &ss)
{
    const yysymbol_kind_t unexpected_token = yypcontext_token(yyctx);
    const YYLTYPE unexpected_token_loc = *utils::require_ptr(yypcontext_location(yyctx));

    if (unexpected_token == YYSYMBOL_YYEMPTY)
    // According to bison manual this mean NO-LOOKAHEAD
        report_no_unexpected_diagnostic(unexpected_token_loc, diagnostic_engine);
    else
    {
        const auto info = Info{
            .unexpected_token = unexpected_token,
            .unexpected_token_name = yysymbol_name(unexpected_token),
            .unexpected_token_source_text = alpha_yyget_text(flex_ctx),
            .unexpected_token_loc = unexpected_token_loc,
            .expected_tokens = collect_expected_tokens(yyctx)
        };
        report_unexpected_diagnostic(lexer_ctx, info, diagnostic_engine);
    }
    return YYREPORT_SYNTAX_ERROR_RETVAL;
}

static void alpha_yyerror(
    const ALPHA_YYLTYPE *const err_loc,
    [[maybe_unused]] const yyscan_t,
    [[maybe_unused]] const alpha::LexerCtx &,
    [[maybe_unused]] const alpha::LocationTracker &,
    alpha::DiagnosticEngine &diagnostic_engine,
    [[maybe_unused]] const alpha::DiagnosticReporter &,
    [[maybe_unused]] const alpha::SemanticSystem &,
    const std::string &error_message)
{
    DEBUG_SMART_ASSERT(
        false && "alpha_yyerror function called why? Memory exhaustion occurred?");
    diagnostic_engine.report(Issue(
        Issue::Type::FATAL_ERROR,
        std::string("ERROR_MESSAGE: ")
        + error_message
        + "\n"
        "INTERNAL-ERROR, IF YOU SEE THIS CONTACT DEVELOPER",
        *err_loc
    ));
}

#endif // PARSER_EPILOGUE_CODE_HPP
