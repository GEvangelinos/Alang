%{
        #include <string>                             // for basic_string, stod, stol
        #include "alpha_parser.hpp"                   // for alpha_yylloc, alpha_yyt...
        #include "core/alpha_error.hpp"               // for CTError, ErrorTracker
        #include "core/alpha_location.hpp"            // for LocationTracker
        #include "core/alpha_shared_interface.hpp"    // for ALPHA_YYLEX_SIGNATURE
        #include "scanner/alpha_scanner_context.hpp"  // for LexerCtx
        #include "scanner/alpha_token.hpp"            // for TokenString, TokenComment
        #include "utils/format_adapter.hpp"           // for format, fmt_ns

        #ifdef YY_DECL
        #undef YY_DECL
        #endif /* YY_DECL */

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

%option noyywrap
%option prefix="alpha_yy"

/* FLex regex definitions */

id              [a-zA-Z][a-zA-Z_0-9]*
integer         (0[xX][0-9a-fA-F]+)|([0-9]+)
real            ([0-9]*\.[0-9]+([eE][\+-]?[0-9]+)?)
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

"if"            {return IF;} 
"else"          {return ELSE;} 
"while"         {return WHILE;}
"for"           {return FOR;} 
"function"      {return FUNCTION;} 
"return"        {return RETURN;} 
"break"         {return BREAK;} 
"continue"      {return CONTINUE;} 
"and"           {return AND;} 
"not"           {return NOT;} 
"or"            {return OR;} 
"local"         {return LOCAL;} 
"true"          {return TRUE;} 
"false"         {return FALSE;} 
"nil"           {return NIL;} 


"="             {return '=';} 
"+"             {return '+';} 
"-"             {return '-';} 
"*"             {return '*';} 
"/"             {return '/';} 
"%"             {return '%';} 
"=="            {return EQ;} 
"!="            {return NEQ;} 
"++"            {return INC;} 
"--"            {return DEC;} 
">"             {return GT;} 
"<"             {return LT;} 
">="            {return GTE;} 
"<="            {return LTE;} 

"{"             {return '{';}
"}"             {return '}';}
"["             {return '[';}
"]"             {return ']';}
"("             {return '(';}
")"             {return ')';}
";"             {return ';';}
","             {return ',';}
":"             {return ':';}
"::"            {return GLOBAL;}
"."             {return '.';}
".."            {return METHOD_CALL;}

{id}            {alpha_yylval.union_id = Alpha::TokenID::refreshLastId(yytext); return ID;}
{real}          {alpha_yylval.union_real_const = std::stod(yytext); return REAL_CONST;}
{integer}       {alpha_yylval.union_int_const = std::stol(yytext); return INT_CONST;}
{lineComment}   {/* Syntax Anal-yzer doesn't give a fuck about you... */}
{newline}       {lt.append_line(lexer_ctx.index_);}
{space}+        {/* Just eat up the spaces. */}

{commentBeginRule} {
        BEGIN(INSIDE_COMMENT_BLOCK);
        Alpha::TokenComment::setStartingLocation(alpha_yylloc);
} /* No return until complete token or EOF. */
<INSIDE_COMMENT_BLOCK>{commentEndRule} {
                BEGIN(INITIAL);
}
<INSIDE_COMMENT_BLOCK>{newline} {
        lt.append_line(lexer_ctx.index_);
}
<INSIDE_COMMENT_BLOCK>{anyCharacterButNewLine} {/* Just eat up any character use see. */}
<INSIDE_COMMENT_BLOCK><<EOF>> {
        et.report_error(
                Alpha::CTError::Type::LEXER,
                "unclosed comment, expected terminating `*/` character",
                alpha_yylloc,
                "comment starts here",
                Alpha::TokenComment::getStartingLocation()
        );
        return EOF;
}

{stringBeginRule} {
        BEGIN(INSIDE_STRING);
        Alpha::TokenString::setStartingLocation(alpha_yylloc);
}
<INSIDE_STRING>{newline} {
        lt.append_line(lexer_ctx.index_);
}
<INSIDE_STRING>{stringValidInnerPart} {
        Alpha::TokenString::appendToAssemblingBuffer(yytext);
}
<INSIDE_STRING>{stringInvalidEscapeCode} {
        Alpha::TokenString::appendToAssemblingBuffer(yytext);
        et.report_error(
                Alpha::CTError::Type::LEXER,
                fmt_ns::format("invalid escape code {} in string", yytext),
                alpha_yylloc
        );
        /* Do not stop scanning, but register lexer error. */
}
<INSIDE_STRING><<EOF>> {
        et.report_error(
                Alpha::CTError::Type::LEXER,
                "unclosed string literal, expected terminating `\"` character",
                alpha_yylloc,
                "string literal starts here",
                Alpha::TokenString::getStartingLocation()
        );
        return EOF;
}
<INSIDE_STRING>{stringEndRule} {
        BEGIN(INITIAL);
        Alpha::TokenString::exportStringToken(&(alpha_yylval.union_string_literal));
        return STRING_LITERAL;
}

{anyCharacterButNewLine} { /* Any other character past this point is considered invalid.*/
        et.report_error(
                Alpha::CTError::Type::LEXER,
                fmt_ns::format("character {} is invalid", yytext),
                alpha_yylloc
        );
}

<<EOF>>          {return ALPHA_YYEOF;}
%%