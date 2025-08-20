#ifndef PARSER_EPILOGUE_CODE_HPP
#define PARSER_EPILOGUE_CODE_HPP

#include <alpha_parser.gen.hpp>
#include <string>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <scanner/alpha_scanner.gen.hpp>

#include "L1_driver/semantic_system.hpp"
#include "scanner/scanner_context.hpp"
#include "utils/debug_tools.hpp"

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
    yypcontext_t const *bison_ctx,
    const yyscan_t flex_ctx,
    [[maybe_unused]] alpha::LexerCtx &lexer_ctx,
    [[maybe_unused]] alpha::LocationTracker &lt,
    alpha::DiagnosticEngine &diagnostic_engine,
    alpha::DiagnosticReporter &dr,
    [[maybe_unused]] alpha::SemanticSystem &ss)
{
    constexpr int retval = 0;
    constexpr int max_tokens = 5; // how many to *list* in multi-expected case

    const yysymbol_kind_t unexpected_token = yypcontext_token(bison_ctx);
    const char *const unexpected_text = alpha_yyget_text(flex_ctx);
    const YYLTYPE *const err_loc = yypcontext_location(bison_ctx);

    std::string syntax_error;

    if (unexpected_token == YYSYMBOL_YYEMPTY) // According to bison manual this mean NO-LOOKAHEAD
    {
        DEBUG_SMART_ASSERT(false &&
            "HOLD YOUR HORSES... You just caused an error,\n"
            "you have no idea how to replicate. Somehow parse\n"
            "encountered an error without a token (an unexpected)\n"
            "Basically if there is no unexpected, it must mean\n"
            "that there is NOTHING to cause an error... Right?\n"
        );
        dr.report_parse_error(
            "Hey a syntax error occured, PLEASE if you see this meesage, "
            "contact the developer and tell him you got this message."
            "Also if you are kind enough, provide him with the source-file "
            "that cause this error. Parser is deterministic,"
            " so he should be able to replicate. THANK YOU",
            *err_loc
        );
        return retval;
    }
    const char *unexpected_token_name = yysymbol_name(unexpected_token);
    if (unexpected_token == YYSYMBOL_ID)
        unexpected_token_name = unexpected_text;

    yysymbol_kind_t expected_tokens[max_tokens];

    const int expected_count = yypcontext_expected_tokens(bison_ctx, expected_tokens, max_tokens);

    auto join_expected = [&](const char *const sep) -> std::string
    {
        std::string out;
        for (auto i = 0; i < expected_count; i++)
        {
            if (i != 0)
                out += sep;
            out += yysymbol_name(expected_tokens[i]);
        }
        return out;
    };

    auto has_expected = [ & ](const yysymbol_kind_t s) -> bool
    {
        yysymbol_kind_t all_expected[100];
        yypcontext_expected_tokens(bison_ctx, all_expected, 100);
        for (auto i = 0; i < 100; i++)
            if (all_expected[i] == s)
                return true;
        return false;
    };

    if (expected_count < 0)
        throw std::runtime_error(ATTACH_CONTEXT("Internal-Error, like memory exhaustion occurred"));
    // === “Too many to list” (Bison returns 0) → generic but clean ===
    if (expected_count == 0)
    {
        if (has_expected(YYSYMBOL_SEMICOLON))
        {
            const alpha::SourceLocation sug_loc = ss.get_loc_of_last_expr();
            const auto primary = alpha::Issue(
                alpha::Issue::Type::SOFT_ERROR,
                FMT::format("unexpected ‘{}’, invalid syntax, did you mean `;`???????",
                            unexpected_token_name),
                *err_loc,
                alpha::Suggestion(";", sug_loc));
            diagnostic_engine.report(primary, {});
            return retval;
        }
        else
            syntax_error = FMT::format(
                "unexpected ‘{}’, invalid syntax", unexpected_token_name
            );
    }
    // === Exactly one expected ===
    else if (expected_count == 1)
        syntax_error = FMT::format(
            "expected1 `{}` before `{}`", yysymbol_name(expected_tokens[0]), unexpected_token_name
        );
        // TODO: The following cases can benefit from heuristics and probabilistic suggestions.
        // === A few expected, list them compactly ==
    else if (expected_count <= max_tokens) // Few expected // TODO: Add simple heuristics..
        syntax_error = FMT::format(
            "expected2 `{}` before `{}`", join_expected(", "), unexpected_token_name
        );

    dr.report_syntax_error(syntax_error, *err_loc);
    return retval;
}

static void alpha_yyerror(
    ALPHA_YYLTYPE *err_loc,
    yyscan_t,
    alpha::LexerCtx &,
    alpha::LocationTracker &,
    alpha::DiagnosticEngine &diagnostic_engine,
    alpha::DiagnosticReporter &dr,
    alpha::SemanticSystem &,
    std::string error_message)
{
    DEBUG_SMART_ASSERT(
        false && "alpha_yyerror function called why? Did memory exhaustion occurred??");
    dr.report_syntax_error(
        std::string("ERROR_MESSAGE: ")
        + error_message
        + " INTERNAL-ERROR, IF YOU SEE THIS CONTACT DEVELOPER"
      , *err_loc
    );
}

#endif // PARSER_EPILOGUE_CODE_HPP
