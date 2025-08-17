%{
        #include <string>                             // for basic_string, stod, stol
        #include <parser/alpha_parser.gen.hpp>                   // for alpha_yylloc, alpha_yyt...
        #include "diagnostics/diagnostic_engine.hpp"               // for CTError, ErrorTracker
        #include "core/source_location.hpp"            // for LocationTracker
        #include "core/shared_interface.hpp"    // for ALPHA_YYLEX_SIGNATURE
        #include "scanner/scanner_context.hpp"  // for LexerCtx
        #include "scanner/scanner_token.hpp"            // for TokenString, TokenComment
        #include "utils/format_adapter.hpp"           // for format, FMT

        #ifdef YY_DECL
        #undef YY_DECL
        #endif

        #define YY_DECL ALPHA_YYLEX_SIGNATURE

        #define YY_USER_ACTION                                                \
                do                                                            \
                {                                                             \
                        alpha_yylloc.first_index = lexer_ctx.index_;         \
                        alpha_yylloc.last_index = lexer_ctx.index_ + yyleng; \
                                                                              \
                        lexer_ctx.index_ += yyleng;                           \
                } while (0); /* Semi-Colon is not placed by flex, we place it manually. */
%}

%option reentrant
%option bison-bridge bison-locations
%option noyywrap
%option prefix="alpha_yy"

/* FLex regex definitions */

id              [a-zA-Z][a-zA-Z_0-9]*
int             (0[xX][0-9a-fA-F]+)|([0-9]+)
float           ([0-9]*\.[0-9]+([eE][\+-]?[0-9]+)?)
lineComment     "//".*
newline         "\n"
space           [ \r\t\v]

anyCharacterButNewLine  .

commentBeginRule        "/\*"
commentEndRule          "\*/"

stringBeginRule         \"
stringEndRule           \"

/* Newlines are valid inside strings, but we exclude '\n' here */
/* so that it matches a separate {newline} rule, allowing us */
/* to update the line tracker explicitly when a line break occurs. */
stringValidInnerPart    ([^\n\"\\]|\\[tn"\\])+

stringInvalidEscapeCode "\\"([^tn"\\])

/* Flex conditions */
%x INSIDE_COMMENT_BLOCK
%x INSIDE_STRING


%%

"if"            { return IF; } 
"else"          { return ELSE; } 
"while"         { return WHILE; }
"for"           { return FOR; } 
"function"      { return FUNCTION; } 
"return"        { return RETURN; } 
"break"         { return BREAK; } 
"continue"      { return CONTINUE; } 
"and"           { return AND; } 
"not"           { return NOT; } 
"or"            { return OR; } 
"local"         { return LOCAL; } 
"true"          { return TRUE; } 
"false"         { return FALSE; } 
"nil"           { return NIL; } 


"="             { return ASSIGN; } 
"+"             { return PLUS; } 
"-"             { return MINUS; } 
"*"             { return MUL; } 
"/"             { return DIV; } 
"%"             { return MOD; } 
"=="            { return EQ; } 
"!="            { return NEQ; } 
"++"            { return INC; } 
"--"            { return DEC; } 
">"             { return GT; } 
"<"             { return LT; } 
">="            { return GTE; } 
"<="            { return LTE; } 

"{"             { return LEFT_BRACE; }
"}"             { return RIGHT_BRACE; }
"["             { return LEFT_BRACKET; }
"]"             { return RIGHT_BRACKET; }
"("             { return LEFT_PAREN; }
")"             { return RIGHT_PAREN; }
";"             { return SEMICOLON; }
","             { return COMMA; }
":"             { return COLON; }
"::"            { return GLOBAL; }
"."             { return DOT; }
".."            { return METHOD_CALL; }

{id}            { alpha_yylval.cstring = alpha::TokenID::refreshLastId(yytext); return ID; } // TODO: probably useless You can just pass the yytext to ID and let parser store it...
{float}         { alpha_yylval.const_float = std::stod(yytext); return FLOAT; }
{int}           { alpha_yylval.const_int = std::stol(yytext); return INT; }
{lineComment}   { /* Syntax Anal-yzer doesn't give a fuck about you... */ }
{newline}       { lt.append_line(lexer_ctx.index_); }
{space}+        { /* Just eat up the spaces. */ }

{commentBeginRule} {
        BEGIN(INSIDE_COMMENT_BLOCK);
        alpha::TokenComment::setStartingLocation(alpha_yylloc);
} /* No return until complete token or EOF. */
<INSIDE_COMMENT_BLOCK>{commentEndRule} {
                BEGIN(INITIAL);
}
<INSIDE_COMMENT_BLOCK>{newline} {
        lt.append_line(lexer_ctx.index_);
}
<INSIDE_COMMENT_BLOCK>{anyCharacterButNewLine} { /* Just eat up any character use see. */ }
<INSIDE_COMMENT_BLOCK><<EOF>> {
        dr.report_unclosed_comment(alpha_yylloc, alpha::TokenComment::getStartingLocation());
        return EOF;
}

{stringBeginRule} {
        BEGIN(INSIDE_STRING);
        alpha::TokenString::setStartingLocation(alpha_yylloc);
}
<INSIDE_STRING>{newline} {
        lt.append_line(lexer_ctx.index_);
}
<INSIDE_STRING>{stringValidInnerPart} {
        alpha::TokenString::appendToAssemblingBuffer(yytext);
}
<INSIDE_STRING>{stringInvalidEscapeCode} {
        alpha::TokenString::appendToAssemblingBuffer(yytext);
        dr.report_invalid_escape_code(yytext, alpha_yylloc);
}
<INSIDE_STRING><<EOF>> {
        dr.report_unclosed_string(alpha_yylloc, alpha::TokenString::getStartingLocation());
        return EOF;
}
<INSIDE_STRING>{stringEndRule} {
        BEGIN(INITIAL);
        alpha::TokenString::exportStringToken(&alpha_yylval.cstring);
        return STRING;
}

{anyCharacterButNewLine} { /* Any other character past this point is considered invalid.*/
        dr.report_invalid_character(yytext, alpha_yylloc);
}

<<EOF>>          {return ALPHA_YYEOF;}
%%