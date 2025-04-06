%{
        #include <errno.h>           // for errno, EINTR
        #include <stdio.h>           // for NULL, FILE, EOF, ferror, size_t, clearerr
        #include <stdlib.h>          // for exit, free, malloc, realloc
        #include <string.h>          // for memset, strlen
        #include <format>
        #include <unistd.h>          // for isatty
        #include <string>            // for allocator, operator+, basic_string, char...
        #include "scanner/alpha_scanner_context.hpp"
        #include "alpha_parser.hpp"   // for alpha_yytokentype, alpha_yylval
        #include "core/alpha_error_tracker.hpp"  // for LexerError, CompileTimeErrorTracker
        #include "scanner/alpha_token.hpp"    // for TokenString, TokenComment, TokenID
        #include "scanner/_internal_alpha_scanner.hpp"
%}

%option noyywrap
/* TODO: is the below line necessary? Bison already defines a prefix, does it count for scanenr too? (probably not) */
/*       as they are different programs. */
%option prefix="alpha_yy"

/* FLex regex definitions */

id              [a-zA-Z][a-zA-Z_0-9]*
integer         (0[xX][0-9a-fA-F]+)|([0-9]+)
real            ([0-9]*\.[0-9]+([eE][\+-]?[0-9]+)?)
lineComment     "//".*
newline         "\n"
space           [ \r\t\v]

anyCharacter            .|\n
anyCharacterButNewLine  .

commentBeginRule        "/\*"
commentEndRule          "\*/"

stringBeginRule         \"
stringEndRule           \"
stringValidInnerPart    ([^\"\\]|\\[tn"\\])+
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
"++"            {return PLUS_PLUS;} 
"--"            {return MINUS_MINUS;} 
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
"::"            {return COLON_BLOCK;}
"."             {return '.';}
".."            {return DDOT;}

{id}            {alpha_yylval.union_id = Alpha::TokenID::refreshLastId(yytext); return ID;}
{real}          {alpha_yylval.union_real_const = std::stod(yytext); return REAL_CONST;}
{integer}       {alpha_yylval.union_int_const = std::stol(yytext); return INT_CONST;}
{lineComment}   {/* Syntax Anal-yzer doesn't give a fuck about you... */}
{newline}*      { scnr_ctx.line_++; }
{space}*        {/* Just eat up the spaces. */}

{commentBeginRule} {
        BEGIN(INSIDE_COMMENT_BLOCK);
        Alpha::TokenComment::setCommentStartingLineNumber(yylineno);
} /* No return until complete token or EOF. */
<INSIDE_COMMENT_BLOCK>{commentBeginRule} {
} /* TODO: What's that? Artifact from PHASE A with nested comments? */
<INSIDE_COMMENT_BLOCK>{commentEndRule} {
                BEGIN(INITIAL);
}
<INSIDE_COMMENT_BLOCK>{anyCharacter} { /* Just eat up any character use see. */}
<INSIDE_COMMENT_BLOCK><<EOF>> {
        error_tracker.register_lexer_error("EOF reached before closing comment.", alpha_yylloc);
        return EOF;
}


{stringBeginRule} {
        BEGIN(INSIDE_STRING);
        Alpha::TokenString::setStringStartingLineNumber(yylineno);
}
<INSIDE_STRING>{stringValidInnerPart} {
        Alpha::TokenString::appendToAssemblingBuffer(yytext);
}
<INSIDE_STRING>{stringInvalidEscapeCode} {
        Alpha::TokenString::appendToAssemblingBuffer(yytext);
        error_tracker.register_lexer_error(
                std::format("Lexer error: Invalid escape code {} in string.", yytext), alpha_yylloc);
        /* Do not stop scanning, but register lexer error. */
}
<INSIDE_STRING><<EOF>> {
        error_tracker.register_lexer_error(
                "Lexer error: EOF reached before closing string.", alpha_yylloc);
        return EOF;
}
<INSIDE_STRING>{stringEndRule} {
        BEGIN(INITIAL);
        Alpha::TokenString::exportStringToken(&(alpha_yylval.union_string_literal));
        return STRING_LITERAL;
}

{anyCharacterButNewLine} { /* Any other character past this point is considered invalid.*/
        error_tracker.register_lexer_error(
                std::format("Lexer error: Character {} is invalid.", yytext), alpha_yylloc);
}

<<EOF>>          {return ALPHA_YYEOF;}
%%