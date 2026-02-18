%{
        #include <string>                             // for basic_string, stod, stol
        #include <parser/alpha_parser.gen.hpp>                   // for alpha_yylloc, alpha_yyt...
        #include "diagnostics/diagnostic_engine.hpp"               // for CTError, ErrorTracker
        #include "core/source_location.hpp"            // for LocationTracker
        #include "core/shared_interface.hpp"    // for ALPHA_YYLEX_SIGNATURE
        #include "scanner/scanner_token.hpp"            // for TokenString, TokenComment
        #include "scanner/scanner_context.hpp"
        #include "support/format_adapter.hpp"           // for format, FMT
        #include <diagnostics/diagnostic_reporter.gen.hpp>
        #include "scanner_prologue.hpp"
%}

%option reentrant
%option bison-bridge bison-locations
%option noyywrap
%option prefix="alpha_yy"

/* FLex regex definitions */

id              [a-zA-Z][a-zA-Z_0-9]*
int             (0[xX][0-9a-fA-F]+)|([0-9]+)
float           ([0-9]*\.[0-9]+([eE][\+-]?[0-9]+)?)
line_comment     "//".*
newline         "\n"
space           [ \r\t\v]

any_character_but_newline  .

comment_begin_rule        "/\*"
comment_end_rule          "\*/"

stringBeginRule         \"
string_end_rule         \"

/* Newlines are valid inside strings, but we exclude '\n' here */
/* so that it matches a separate {newline} rule, allowing us */
/* to update the line tracker explicitly when a line break occurs. */
string_valid_inner_part    ([^\n\"\\]|\\[tn"\\])+

string_invalid_escape_code "\\"([^tnr"\\])

/* Flex conditions */
%x INSIDE_COMMENT_BLOCK
%x INSIDE_STRING


%%

"if"            { YIELD_TOKEN(TKN_IF); }
"else"          { YIELD_TOKEN(TKN_ELSE); }
"while"         { YIELD_TOKEN(TKN_WHILE); }
"for"           { YIELD_TOKEN(TKN_FOR); }
"function"      { YIELD_TOKEN(TKN_FUNCTION); }
"return"        { YIELD_TOKEN(TKN_RETURN); }
"break"         { YIELD_TOKEN(TKN_BREAK); }
"continue"      { YIELD_TOKEN(TKN_CONTINUE); }
"not"           { YIELD_TOKEN(TKN_NOT); }
"and"           { YIELD_TOKEN(TKN_AND); }
"or"            { YIELD_TOKEN(TKN_OR); }
"local"         { YIELD_TOKEN(TKN_LOCAL); }
"true"          { YIELD_TOKEN(TKN_TRUE); }
"false"         { YIELD_TOKEN(TKN_FALSE); }
"nil"           { YIELD_TOKEN(TKN_NIL); }

"="             { YIELD_TOKEN(TKN_ASSIGN); }
"+"             { YIELD_TOKEN(TKN_PLUS); }
"-"             { YIELD_TOKEN(TKN_MINUS); }
"*"             { YIELD_TOKEN(TKN_MUL); }
"/"             { YIELD_TOKEN(TKN_DIV); }
"%"             { YIELD_TOKEN(TKN_MOD); }
"=="            { YIELD_TOKEN(TKN_EQ); }
"!="            { YIELD_TOKEN(TKN_NEQ); }
"++"            { YIELD_TOKEN(TKN_INC); }
"--"            { YIELD_TOKEN(TKN_DEC); }
">"             { YIELD_TOKEN(TKN_GT); }
"<"             { YIELD_TOKEN(TKN_LT); }
">="            { YIELD_TOKEN(TKN_GTE); }
"<="            { YIELD_TOKEN(TKN_LTE); }

"{"             { YIELD_TOKEN(TKN_LEFT_BRACE); }
"}"             { YIELD_TOKEN(TKN_RIGHT_BRACE); }
"["             { YIELD_TOKEN(TKN_LEFT_BRACKET); }
"]"             { YIELD_TOKEN(TKN_RIGHT_BRACKET); }
"("             { YIELD_TOKEN(TKN_LEFT_PAREN); }
")"             { YIELD_TOKEN(TKN_RIGHT_PAREN); }
";"             { YIELD_TOKEN(TKN_SEMICOLON); }
","             { YIELD_TOKEN(TKN_COMMA); }
":"             { YIELD_TOKEN(TKN_COLON); }
"::"            { YIELD_TOKEN(TKN_GLOBAL); }
"."             { YIELD_TOKEN(TKN_DOT); }
".."            { YIELD_TOKEN(TKN_METHOD_CALL); }

{id}            { yylval->cstring = alpha::TokenID::refresh_last_id(yytext); YIELD_TOKEN(TKN_ID); } // TODO: probably useless You can just pass the yytext to ID and let parser store it...
{float}         { yylval->const_float = std::stod(yytext); YIELD_TOKEN(TKN_FLOAT); }
{int}           { yylval->const_int = std::stol(yytext); YIELD_TOKEN(TKN_INT); }
{line_comment}   { /* Syntax Anal-yzer doesn't give a fuck about you... */ }
{newline}       { lt.append_line(lexer_ctx.source_index); }
{space}+        { /* Just eat up the spaces. */ }

{comment_begin_rule} {
        BEGIN(INSIDE_COMMENT_BLOCK);
        alpha::TokenComment::set_starting_location(*yylloc);
} /* No return until complete token or EOF. */
<INSIDE_COMMENT_BLOCK>{comment_end_rule} {
                BEGIN(INITIAL);
}
<INSIDE_COMMENT_BLOCK>{newline} {
        lt.append_line(lexer_ctx.source_index);
}
<INSIDE_COMMENT_BLOCK>{any_character_but_newline} { /* Just eat up any character use see. */ }
<INSIDE_COMMENT_BLOCK><<EOF>> {
        dr.report_unclosed_comment(alpha::TokenComment::get_starting_location());
        YIELD_TOKEN(TKN_YYEOF);
}

{stringBeginRule} {
        BEGIN(INSIDE_STRING);
        alpha::TokenString::set_starting_location(*yylloc);
}
<INSIDE_STRING>{newline} {
        lt.append_line(lexer_ctx.source_index);
}
<INSIDE_STRING>{string_valid_inner_part} {
        alpha::TokenString::append_to_assembling_buffer(yytext);
}
<INSIDE_STRING>{string_invalid_escape_code} {
        alpha::TokenString::append_to_assembling_buffer(yytext);
        dr.report_invalid_escape_code(yytext, *yylloc);
}
<INSIDE_STRING><<EOF>> {
        dr.report_unclosed_string(alpha::TokenString::get_starting_location());
        YIELD_TOKEN(TKN_YYEOF);
}
<INSIDE_STRING>{string_end_rule} {
        BEGIN(INITIAL);
        yylval->cstring =  alpha::TokenString::export_string_token();
        *yylloc = alpha::TokenString::export_location(*yylloc);
        YIELD_TOKEN(TKN_STRING);
}

{any_character_but_newline} { /* Any other character past this point is considered invalid. */
        dr.report_invalid_character(yytext, *yylloc);
}

<<EOF>>          { YIELD_TOKEN(TKN_YYEOF); }
%%