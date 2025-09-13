%{
        #include <string>                             // for basic_string, stod, stol
        #include <parser/alpha_parser.gen.hpp>                   // for alpha_yylloc, alpha_yyt...
        #include "diagnostics/diagnostic_engine.hpp"               // for CTError, ErrorTracker
        #include "core/source_location.hpp"            // for LocationTracker
        #include "core/shared_interface.hpp"    // for ALPHA_YYLEX_SIGNATURE
        #include "scanner/scanner_token.hpp"            // for TokenString, TokenComment
        #include "scanner/scanner_context.hpp"
        #include "utils/format_adapter.hpp"           // for format, FMT
        #include <diagnostics/diagnostic_reporter.gen.hpp>

        #define YY_USER_ACTION                                                \
                do                                                            \
                {                                                             \
                        yylloc->first_index = lexer_ctx.index_;               \
                        yylloc->last_index = lexer_ctx.index_ + yyleng;       \
                                                                              \
                        lexer_ctx.index_ += yyleng;                           \
                } while (0); /* Semi-Colon is not placed by flex, we place it manually. */
        #define EMIT(TOK_ID)                                                                      \
                do                                                                                \
                {                                                                                 \
                        lexer_ctx.register_token(alpha::TokenInfo{.id = TOK_ID, .loc = *yylloc}); \
                        return TOK_ID;                                                            \
                } while(0)
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
string_end_rule           \"

/* Newlines are valid inside strings, but we exclude '\n' here */
/* so that it matches a separate {newline} rule, allowing us */
/* to update the line tracker explicitly when a line break occurs. */
string_valid_inner_part    ([^\n\"\\]|\\[tn"\\])+

string_invalid_escape_code "\\"([^tnr"\\])

/* Flex conditions */
%x INSIDE_COMMENT_BLOCK
%x INSIDE_STRING


%%

"if"            { EMIT(IF); } 
"else"          { EMIT(ELSE); } 
"while"         { EMIT(WHILE); }
"for"           { EMIT(FOR); } 
"function"      { EMIT(FUNCTION); } 
"return"        { EMIT(RETURN); } 
"break"         { EMIT(BREAK); } 
"continue"      { EMIT(CONTINUE); } 
"and"           { EMIT(AND); } 
"not"           { EMIT(NOT); } 
"or"            { EMIT(OR); } 
"local"         { EMIT(LOCAL); } 
"true"          { EMIT(TRUE); } 
"false"         { EMIT(FALSE); } 
"nil"           { EMIT(NIL); } 


"="             { EMIT(ASSIGN); } 
"+"             { EMIT(PLUS); } 
"-"             { EMIT(MINUS); } 
"*"             { EMIT(MUL); } 
"/"             { EMIT(DIV); } 
"%"             { EMIT(MOD); } 
"=="            { EMIT(EQ); } 
"!="            { EMIT(NEQ); } 
"++"            { EMIT(INC); } 
"--"            { EMIT(DEC); } 
">"             { EMIT(GT); } 
"<"             { EMIT(LT); } 
">="            { EMIT(GTE); } 
"<="            { EMIT(LTE); } 

"{"             { EMIT(LEFT_BRACE); }
"}"             { EMIT(RIGHT_BRACE); }
"["             { EMIT(LEFT_BRACKET); }
"]"             { EMIT(RIGHT_BRACKET); }
"("             { EMIT(LEFT_PAREN); }
")"             { EMIT(RIGHT_PAREN); }
";"             { EMIT(SEMICOLON); }
","             { EMIT(COMMA); }
":"             { EMIT(COLON); }
"::"            { EMIT(GLOBAL); }
"."             { EMIT(DOT); }
".."            { EMIT(METHOD_CALL); }

{id}            { yylval->cstring = alpha::TokenID::refresh_last_id(yytext); EMIT(ID); } // TODO: probably useless You can just pass the yytext to ID and let parser store it...
{float}         { yylval->const_float = std::stod(yytext); EMIT(FLOAT); }
{int}           { yylval->const_int = std::stol(yytext); EMIT(INT); }
{line_comment}   { /* Syntax Anal-yzer doesn't give a fuck about you... */ }
{newline}       { lt.append_line(lexer_ctx.index_); }
{space}+        { /* Just eat up the spaces. */ }

{comment_begin_rule} {
        BEGIN(INSIDE_COMMENT_BLOCK);
        alpha::TokenComment::set_starting_location(*yylloc);
} /* No return until complete token or EOF. */
<INSIDE_COMMENT_BLOCK>{comment_end_rule} {
                BEGIN(INITIAL);
}
<INSIDE_COMMENT_BLOCK>{newline} {
        lt.append_line(lexer_ctx.index_);
}
<INSIDE_COMMENT_BLOCK>{any_character_but_newline} { /* Just eat up any character use see. */ }
<INSIDE_COMMENT_BLOCK><<EOF>> {
        dr.report_unclosed_comment(*yylloc, alpha::TokenComment::get_starting_location());
        return ALPHA_YYEOF;
}

{stringBeginRule} {
        BEGIN(INSIDE_STRING);
        alpha::TokenString::set_starting_location(*yylloc);
}
<INSIDE_STRING>{newline} {
        lt.append_line(lexer_ctx.index_);
}
<INSIDE_STRING>{string_valid_inner_part} {
        alpha::TokenString::append_to_assembling_buffer(yytext);
}
<INSIDE_STRING>{string_invalid_escape_code} {
        alpha::TokenString::append_to_assembling_buffer(yytext);
        dr.report_invalid_escape_code(yytext, *yylloc);
}
<INSIDE_STRING><<EOF>> {
        dr.report_unclosed_string(*yylloc, alpha::TokenString::get_starting_location());
        return ALPHA_YYEOF;
}
<INSIDE_STRING>{string_end_rule} {
        BEGIN(INITIAL);
        yylval->cstring =  alpha::TokenString::export_string_token();
        EMIT(STRING);
}

{any_character_but_newline} { /* Any other character past this point is considered invalid. */
        dr.report_invalid_character(yytext, *yylloc);
}

<<EOF>>          { return ALPHA_YYEOF; }
%%