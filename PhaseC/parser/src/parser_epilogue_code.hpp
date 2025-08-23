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
constexpr Issue::Type SYNTAX_ERROR_ISSUE_TYPE = Issue::Type::SOFT_ERROR;
constexpr unsigned FEW_TOKENS = 5;
constexpr int YYREPORT_SYNTAX_ERROR_RETVAL = 0;

struct Info
{
    std::vector<yysymbol_kind_t> expected_tokens;
    const yysymbol_kind_t unexpected_token;
    const char *const unexpected_token_name;
    const char *const unexpected_token_source_text;
    const YYLTYPE unexpected_token_loc;
};

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
        out += ' ' + '\'' + info.unexpected_token_source_text + '\'';
        break;
    case YYSYMBOL_STRING:
        out += ' ' + '\"' + info.unexpected_token_source_text + '\"';
        break;
    }
    return out;
}

[[nodiscard]] static Issue
make_no_expected_issue(const Info &info)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.empty());

    return Issue(
        SYNTAX_ERROR_ISSUE_TYPE,
        FMT::format("unexpected `{}`", get_formatted_unexpected_token_name(info)),
        info.unexpected_token_loc
    );
}

[[nodiscard]] static Issue
make_few_expected_issue(const LexerCtx &lexer_ctx, const Info &info
)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.size() <= FEW_TOKENS);
    auto join_expected = [&](const char *const sep) -> std::string
    {
        std::string out;
        for (auto i = 0; i < info.expected_tokens.size(); i++)
        {
            if (i != 0)
                out += sep;
            out += yysymbol_name(info.expected_tokens[i]);
        }
        return out;
    };

    std::optional<Suggestion> suggestion;
    if (const auto token_info = lexer_ctx.get_second_last_token(); token_info.has_value())
        suggestion.emplace(join_expected("\n"), token_info->loc);

    return Issue(
        SYNTAX_ERROR_ISSUE_TYPE,
        FMT::format("expected `{}` before `{}`", join_expected(", "),
                    get_formatted_unexpected_token_name(info)),
        info.unexpected_token_loc,
        suggestion
    );
}

[[nodiscard]] static Issue
make_too_many_expected_issue(const LexerCtx &lexer_ctx, const Info &info)
{
    DEBUG_SMART_ASSERT(
        info.unexpected_token != YYSYMBOL_YYEMPTY && "No Lookahead, shouldn't be called");
    DEBUG_SMART_ASSERT(info.expected_tokens.size() > FEW_TOKENS);

    auto has_expected = [&](const yysymbol_kind_t s)
    {
        for (auto i = 0; i < info.expected_tokens.size(); ++i)
            if (info.expected_tokens[i] == s)
                return true;
        return false;
    };

    // Priority: ;  then ) ] }
    yysymbol_kind_t suggestion_pick = YYSYMBOL_YYEMPTY;
    if (has_expected(YYSYMBOL_SEMICOLON)) suggestion_pick = YYSYMBOL_SEMICOLON;
    else if (has_expected(YYSYMBOL_RIGHT_PAREN)) suggestion_pick = YYSYMBOL_RIGHT_PAREN;
    else if (has_expected(YYSYMBOL_RIGHT_BRACKET)) suggestion_pick = YYSYMBOL_RIGHT_BRACKET;
    else if (has_expected(YYSYMBOL_RIGHT_BRACE)) suggestion_pick = YYSYMBOL_RIGHT_BRACE;

    std::optional<Suggestion> suggestion = std::nullopt;

    if (suggestion_pick != YYSYMBOL_YYEMPTY)
        if (const auto token_info = lexer_ctx.get_second_last_token(); token_info.has_value())
            suggestion.emplace(yysymbol_name(suggestion_pick), token_info->loc);

    return Issue(
        SYNTAX_ERROR_ISSUE_TYPE,
        FMT::format("unexpected ‘{}’, invalid syntax", get_formatted_unexpected_token_name(info)),
        info.unexpected_token_loc,
        suggestion
    );
}

[[nodiscard]] static Issue
make_no_unexpected_issue(const YYLTYPE unexpected_loc)
{
    DEBUG_SMART_ASSERT(false &&
        "HOLD YOUR HORSES... You just caused an error,\n"
        "you have no idea how to replicate. Somehow parse\n"
        "encountered an error without a token (an unexpected)\n"
        "Basically if there is no unexpected, it must mean\n"
        "that there is NOTHING to cause an error... Right?\n"
    );
    return Issue(
        Issue::Type::FATAL_ERROR,
        "Hey a syntax error occurred, PLEASE if you see this message, "
        "contact the developer and tell him you got this message."
        "Also if you are kind enough, provide him with the source-file "
        "that cause this error. Parser is deterministic,"
        "so he should be able to replicate. THANK YOU",
        unexpected_loc
    );
}

[[nodiscard]] static Issue
make_unexpected_issue(const LexerCtx &lexer_ctx, const Info &info)
{
    if (info.expected_tokens.empty())
        return make_no_expected_issue(info);
    if (info.expected_tokens.size() <= FEW_TOKENS)
        return make_few_expected_issue(lexer_ctx, info);
    return make_too_many_expected_issue(lexer_ctx, info);
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
    const YYLTYPE *const unexpected_token_loc = yypcontext_location(bison_ctx);

    if (unexpected_token == YYSYMBOL_YYEMPTY) // According to bison manual this mean NO-LOOKAHEAD
        diagnostic_engine.report(make_no_unexpected_issue(unexpected_token_loc));
    else
    {
        const auto info = Info{
            .unexpected_token = unexpected_token,
            .unexpected_token_loc = unexpected_token_loc,
            .unexpected_token_name = yysymbol_name(unexpected_token),
            .unexpected_token_source_text = alpha_yyget_text(flex_ctx),
            .expected_tokens = collect_expected_tokens(yyctx)
        };
        diagnostic_engine.report(make_unexpected_issue(lexer_ctx, info));
    }
    return YYREPORT_SYNTAX_ERROR_RETVAL;
}

static void alpha_yyerror(
    const ALPHA_YYLTYPE *const err_loc,
    yyscan_t,
    alpha::LexerCtx &,
    alpha::LocationTracker &,
    alpha::DiagnosticEngine &diagnostic_engine,
    alpha::DiagnosticReporter &dr,
    alpha::SemanticSystem &,
    const std::string &error_message)
{
    DEBUG_SMART_ASSERT(false && "alpha_yyerror function called why? Memory exhaustion occurred?");
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
