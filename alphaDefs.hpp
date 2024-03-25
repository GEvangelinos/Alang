#ifndef ALPHA_DEFS_HPP
#define ALPHA_DEFS_HPP

#if defined(INSIDE_FLEX_FILE)
#define YY_DECL int alpha_yylex(void *yylval)

#define ALPHA_EXPORT_KEYWORD_TOKEN(KEYWORD_CODE)                     \
        Alpha::Token::exportToken(yylval, yylineno, yytext, #KEYWORD_CODE); \
        return static_cast<int>(Alpha::KeywordCodes::KEYWORD_CODE)
#define ALPHA_EXPORT_OPERATOR_TOKEN(OPERATOR_CODE)                   \
        Alpha::Token::exportToken(yylval, yylineno, yytext, #OPERATOR_CODE); \
        return static_cast<int>(Alpha::OperatorCodes::OPERATOR_CODE)
#define ALPHA_EXPORT_PUNCTUATION_TOKEN(PUNCTUATION_CODE)             \
        Alpha::Token::exportToken(yylval, yylineno, yytext, #PUNCTUATION_CODE); \
        return static_cast<int>(Alpha::PunctuationCodes::PUNCTUATION_CODE)

#else
int alpha_yylex(void *yylval);
#endif /* FLEX_FILE */

#endif /* ALPHA_DEFS_HPP */