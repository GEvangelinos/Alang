%{
        #define INSIDE_FLEX_FILE

        #include <iostream>
        #include "alphaDefs.hpp" 
        #include "alphaLang.hpp"
        #include "alphaTokenCodes.hpp"
        #include "alphaExceptions.hpp"
%}


/* Flex set options. */
%option yylineno 
%option noyywrap
%option header-file="alphaFlexScanner.hpp"
%option outfile="alphaFlexScanner.cpp"

/* FLex regex definitions */

id              [a-zA-Z][a-zA-Z_0-9]*
integer         (0[xX][0-9a-fA-F]+)|([0-9]+)
real            ([0-9]*\.[0-9]+([eE][\+-]?[0-9]+)?)
lineComment    "//".*
newline         "\n"
space           [ \r\t\v]

anyCharacter            .|\n
anyCharacterButNewLine  .

commentStart       "/\*"
commentEnd         "\*/"

/* Flex conditions */
%x COMMENT_BLOCK


%%

"if"            {ALPHA_EXPORT_KEYWORD_TOKEN(IF); } 
"else"          {ALPHA_EXPORT_KEYWORD_TOKEN(ELSE);} 
"while"         {ALPHA_EXPORT_KEYWORD_TOKEN(WHILE);} 
"for"           {ALPHA_EXPORT_KEYWORD_TOKEN(FOR);} 
"function"      {ALPHA_EXPORT_KEYWORD_TOKEN(FUNCTION);} 
"return"        {ALPHA_EXPORT_KEYWORD_TOKEN(RETURN);} 
"break"         {ALPHA_EXPORT_KEYWORD_TOKEN(BREAK);} 
"continue"      {ALPHA_EXPORT_KEYWORD_TOKEN(CONTINUE);} 
"and"           {ALPHA_EXPORT_KEYWORD_TOKEN(AND);} 
"not"           {ALPHA_EXPORT_KEYWORD_TOKEN(NOT);} 
"or"            {ALPHA_EXPORT_KEYWORD_TOKEN(OR);} 
"local"         {ALPHA_EXPORT_KEYWORD_TOKEN(LOCAL);} 
"true"          {ALPHA_EXPORT_KEYWORD_TOKEN(TRUE);} 
"false"         {ALPHA_EXPORT_KEYWORD_TOKEN(FALSE);} 
"nil"           {ALPHA_EXPORT_KEYWORD_TOKEN(NIL);} 


"="             {ALPHA_EXPORT_OPERATOR_TOKEN(ASSIGN);} 
"+"             {ALPHA_EXPORT_OPERATOR_TOKEN(PLUS);} 
"-"             {ALPHA_EXPORT_OPERATOR_TOKEN(MINUS);} 
"*"             {ALPHA_EXPORT_OPERATOR_TOKEN(MUL);} 
"/"             {ALPHA_EXPORT_OPERATOR_TOKEN(DIV);} 
"%"             {ALPHA_EXPORT_OPERATOR_TOKEN(MOD);} 
"=="            {ALPHA_EXPORT_OPERATOR_TOKEN(EQUAL);} 
"!="            {ALPHA_EXPORT_OPERATOR_TOKEN(NOT_EQUAL);} 
"++"            {ALPHA_EXPORT_OPERATOR_TOKEN(PLUS_PLUS);} 
"--"            {ALPHA_EXPORT_OPERATOR_TOKEN(MINUS_MINUS);} 
">"             {ALPHA_EXPORT_OPERATOR_TOKEN(GREATER_THAN);} 
"<"             {ALPHA_EXPORT_OPERATOR_TOKEN(LESS_THAN);} 
">="            {ALPHA_EXPORT_OPERATOR_TOKEN(GREATER_THAN_OR_EQUAL);} 
"<="            {ALPHA_EXPORT_OPERATOR_TOKEN(LESS_THAN_OR_EQUAL);} 

"{"             {ALPHA_EXPORT_PUNCTUATION_TOKEN(LEFT_BRACE);}
"}"             {ALPHA_EXPORT_PUNCTUATION_TOKEN(RIGHT_BRACE);}
"["             {ALPHA_EXPORT_PUNCTUATION_TOKEN(LEFT_BRACKET);}
"]"             {ALPHA_EXPORT_PUNCTUATION_TOKEN(RIGHT_BRACKET);}
"("             {ALPHA_EXPORT_PUNCTUATION_TOKEN(LEFT_PARENTHESIS);}
")"             {ALPHA_EXPORT_PUNCTUATION_TOKEN(RIGHT_PARENTHESIS);}
";"             {ALPHA_EXPORT_PUNCTUATION_TOKEN(SEMI_COLON);}
","             {ALPHA_EXPORT_PUNCTUATION_TOKEN(COMMA);}
":"             {ALPHA_EXPORT_PUNCTUATION_TOKEN(COLON);}
"::"            {ALPHA_EXPORT_PUNCTUATION_TOKEN(COLON_BLOCK);}
"."             {ALPHA_EXPORT_PUNCTUATION_TOKEN(DOT);}
".."            {ALPHA_EXPORT_PUNCTUATION_TOKEN(DDOT);}

{id}            {Alpha::Token::exportToken(yylval, yylineno, yytext, yytext); return Alpha::idGroupCode;}
{real}          {Alpha::Token::exportToken(yylval, yylineno, yytext, yytext); return Alpha::realNumberGroupCode;}
{integer}       {Alpha::Token::exportToken(yylval, yylineno, yytext, yytext); return Alpha::integerNumberGroupCode;}
{lineComment}   {Alpha::TokenComment::exportLineCommentToken(yylval, yylineno); return static_cast<int>(Alpha::CommentTypeCodes::LINE);}
{newline}*      {/* Just eat up the new line. */}
{space}*        {/* Just eat up the spaces. */}
{commentStart} {
        BEGIN(COMMENT_BLOCK);
        Alpha::TokenComment::addStartLineOfBlockComment(yylineno);
} /* No return until complete token or EOF. */

<COMMENT_BLOCK>{commentStart} {
        Alpha::TokenComment::addStartLineOfBlockComment(yylineno);
}
<COMMENT_BLOCK>{commentEnd} {
        Alpha::TokenComment::exportBlockCommentToken(yylval, yylineno);
        if (Alpha::TokenComment::isInNestedState() == false)
                BEGIN(INITIAL);
        return Alpha::TokenComment::isInNestedState() ? static_cast<int>(Alpha::CommentTypeCodes::NESTED) : static_cast<int>(Alpha::CommentTypeCodes::BLOCK);
}
<COMMENT_BLOCK>{anyCharacter} { /* Just eat up any character use see. */}
<COMMENT_BLOCK><<EOF>> {
        throw Alpha::BlockCommentEOF("EOF reached while being inside a NESTED/BLOCK COMMENT");
}




                 
                 /* Any other character past this point is considered invalid.  DO NOT PUT THIS COMMENT AT THE BEGINNING OF THE LINE. */
{anyCharacterButNewLine} {
        Alpha::Token::exportToken(yylval, yylineno, yytext, yytext);
        return Alpha::invalidCharacterGroupCode;
}

<<EOF>>          {return ALPHA_YYLEX_EOF;}
%%