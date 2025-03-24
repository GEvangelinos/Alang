%{
        
        #define INSIDE_FLEX_FILE
        #include <errno.h>           // for errno, EINTR
        #include <stdio.h>           // for NULL, FILE, EOF, ferror, size_t, clearerr
        #include <stdlib.h>          // for exit, free, malloc, realloc
        #include <string.h>          // for memset, strlen
        #include <unistd.h>          // for isatty
        #include <string>            // for allocator, operator+, basic_string, char...
        #include "alphaParser.hpp"   // for alpha_yytokentype, alpha_yylval
        #include "alphaToken.hpp"    // for TokenString, TokenComment, TokenID
        #include "errorTracker.hpp"  // for LexerError, CompileTimeErrorTracker
        #include "symbolTable.hpp"   // for SymbolTable


        extern Alpha::SymbolTable symbolTable;
%}


/* Flex set options. */
%option yylineno 
%option noyywrap
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

{id}            {alpha_yylval.unionId = Alpha::TokenID::refreshLastId(yytext); return ID;}
{real}          {alpha_yylval.unionRealConst = std::stod(yytext); return REAL_CONST;}
{integer}       {alpha_yylval.unionIntConst = std::stol(yytext); return INT_CONST;}
{lineComment}   {/*Syntax Anal-yzer doesn't give a fuck about you... */}
{newline}*      {/* Just eat up the new line. */}
{space}*        {/* Just eat up the spaces. */}

{commentBeginRule} {
        BEGIN(INSIDE_COMMENT_BLOCK);
        Alpha::TokenComment::setCommentStartingLineNumber(yylineno);
} /* No return until complete token or EOF. */
<INSIDE_COMMENT_BLOCK>{commentBeginRule} {
} /* Artifact from PHASE A with nested comments? */
<INSIDE_COMMENT_BLOCK>{commentEndRule} {
                BEGIN(INITIAL);
}
<INSIDE_COMMENT_BLOCK>{anyCharacter} { /* Just eat up any character use see. */}
<INSIDE_COMMENT_BLOCK><<EOF>> {
        symbolTable.errorTracker.registerCompileTimeError(new Alpha::LexerError(
                yylineno, 0, std::string("Lexer error: EOF reached before closing comment.") + \
        " Comment begun in line: " + std::to_string(Alpha::TokenComment::getCommentStartingLineNumber())));
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
        symbolTable.errorTracker.registerCompileTimeError(new Alpha::LexerError(
                yylineno, 0, std::string("Lexer error: Invalid escape code ") + yytext + " in string."));
        /* Do not stop scanning, but register lexer error. */
}
<INSIDE_STRING><<EOF>> {
        symbolTable.errorTracker.registerCompileTimeError(new Alpha::LexerError(
                yylineno, 0, std::string("Lexer error: EOF reached before closing string.") +\
        " String begun in line: " + std::to_string(Alpha::TokenString::getStringStartingLineNumber())));
        return EOF;
}
<INSIDE_STRING>{stringEndRule} {
        BEGIN(INITIAL);
        Alpha::TokenString::exportStringToken(&(alpha_yylval.unionStringLiteral));
        return STRING_LITERAL;
}

{anyCharacterButNewLine} { /* Any other character past this point is considered invalid.*/
        symbolTable.errorTracker.registerCompileTimeError(new Alpha::LexerError(
                yylineno, 0, std::string("Lexer error: Character ") + yytext + " is invalid."));
}

<<EOF>>          {return ALPHA_YYEOF;}
%%