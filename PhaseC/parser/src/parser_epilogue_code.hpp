#ifndef PARSER_EPILOGUE_CODE_HPP
#define PARSER_EPILOGUE_CODE_HPP
#include <scanner/alpha_scanner.gen.hpp>

static int yyreport_syntax_error(
    yypcontext_t const *bison_ctx,
    yyscan_t flex_ctx,
    [[maybe_unused]] alpha::LexerCtx &,
    [[maybe_unused]] alpha::LocationTracker &,
    alpha::DiagnosticReporter &dr,
    [[maybe_unused]] alpha::SemanticSystem &)
{
    // Always return 0 (success).
    // Per the Bison manual, yyreport_syntax_error should return 0 or YYENOMEM
    // on allocation failure. Our driver handles allocation failures via C++
    // exceptions instead. Additionally, the generated parser we target (Bison
    // 3.8.2 on Fedora) checks a hard-coded literal `2` rather than `YYENOMEM`
    // when propagating memory exhaustion from yyreport_syntax_error. To avoid
    // coupling to that quirk and to keep behavior stable across versions, we
    // do not signal exhaustion via the return value. If an allocation fails in
    // this function, we throw; otherwise we emit diagnostics and return 0.
    constexpr int retval = 0;
    constexpr int max_token = 1000;

    const yysymbol_kind_t unexpected_token = yypcontext_token(bison_ctx);
    const char *const unexpected_token_text = alpha_yyget_text(flex_ctx);
    const YYLTYPE *const unexpected_token_loc = yypcontext_location(bison_ctx);

    std::string syntax_error;

    if (unexpected_token == YYSYMBOL_YYEMPTY) // According to bison manual this mean NO-LOOKAHEAD
    {
        dr.report_parse_error("Lookahed Correction algorithm, failed finding unexpected token)",
                              *unexpected_token_loc);
        return retval;
    }
    const char *unexpected_token_name = yysymbol_name(unexpected_token);
    if (unexpected_token == YYSYMBOL_ID)
        unexpected_token_name = unexpected_token_text;

    yysymbol_kind_t expected_tokens[max_token];

    const auto passed_token = yypcontext_expected_tokens(bison_ctx, expected_tokens, max_token);

    auto symbol_name_join = [passed_token,&expected_tokens](const char *const joint) -> std::string
    {
        std::string comma_sep_tokens;
        for (auto i = 0; i < passed_token; i++)
        {
            if (i != 0)
                comma_sep_tokens += joint;
            comma_sep_tokens += yysymbol_name(expected_tokens[i]);
        }
        return comma_sep_tokens;
    };

    if (passed_token == 0) // Per bison doc, this means more than max_token (argc) matches existed.
        syntax_error = FMT::format(
            "Unexpected `{}` | Lookahead Correction algorith, matched more than {} expected tokens",
            unexpected_token_name, max_token
        );
    else if (passed_token == 1)
        syntax_error = FMT::format(
            " Unexpected `{}` | Lookahead Correction algorith, matched more than {} expected tokens",
            unexpected_token_name, max_token
        );
    else if (passed_token <= max_token)
        syntax_error = FMT::format(
            "Unexpected `{}` | LookAhead Correction algorithm, could match (expected) the following #{} tokens [{}]\n"
            "YYTEXT == {}",
            unexpected_token_name, passed_token, symbol_name_join(","), alpha_yyget_text(flex_ctx)
        );
    else
        syntax_error = FMT::format("Unexpected `{}` | Unknown syntax error", unexpected_token_name);

    dr.report_parse_error(syntax_error, *unexpected_token_loc);
    return retval;
}

#endif // PARSER_EPILOGUE_CODE_HPP
