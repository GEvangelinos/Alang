%{
        #define INSIDE_BISON_FILE
        #define __LOG__ ((void)0); /* Hook for python script to add logging. */
        #include <string>
        #include <list>
        #include <iostream>
        #include <stdexcept>
        #include "alphaScanner.hpp"
        #include "alphaDefs.hpp"
        #include "errorTracker.hpp"
        #include "semantic_actions.hpp"
        bool isFunctionBlock = false;
        bool lvalueIsMember = false;
        int functionDepthCounter = 0;
%}

%code requires
{
        #include "symbolTable.hpp"
        extern Alpha::SymbolTable symbolTable;
}

%define api.prefix {alpha_yy}
%define parse.error detailed    /* Enable detailed error messages */


%start program

%union{
        char *unionStringLiteral;
        char *unionId;
        long unionIntConst;
        double unionRealConst;
        Alpha::SymbolTableEntry *unionLvalue;
}

%token <unionStringLiteral> STRING_LITERAL
%token <unionId>ID
%token <unionIntConst> INT_CONST
%token <unionRealConst> REAL_CONST
%type <unionLvalue> lvalue

/* Keyword tokens */
%token IF          ELSE           WHILE   FOR
%token FUNCTION    RETURN         BREAK   CONTINUE
%token AND         NOT            OR      LOCAL
%token TRUE        FALSE          NIL

/* Operator tokens */
%token '='      '+'     '-'     '*'     '/'     '%'
%token EQ       NEQ      GT      LT      GTE     LTE
%token MINUS_MINUS      PLUS_PLUS         

/* Punctuation tokens */
%token '{'  '}'  '['  ']'  '('  ')'
%token ';'  ','  ':'  '.'  DDOT COLON_BLOCK

/* Priorities */
%right '='

%left OR
%left AND

%nonassoc EQ NEQ
%nonassoc GT GTE LT LTE

%left '+' '-'
%left '*' '/' '%'

%right NOT PLUS_PLUS MINUS_MINUS UMINUS

%left '.' DDOT

%left '[' ']'
%left '(' ')'

%precedence THEN
%precedence ELSE

/* Grammar rules: */

%%
program:
             { __LOG__; }
| multi_stmt { __LOG__; }
;

stmt:   
  expr ';'     { __LOG__ }
| ifstmt              { __LOG__ }
| whilestmt           { __LOG__ }
| forstmt             { __LOG__ }
| returnstmt          { __LOG__ }
| BREAK ';'    { stmt__break_semicolon(); __LOG__ }
| CONTINUE ';' { stmt__continue_semicolon(); __LOG__ }
| block               { __LOG__ }
| funcdef             { __LOG__ }
| ';'          { __LOG__ }
;

multi_stmt:
  stmt            { __LOG__ }
| stmt multi_stmt { __LOG__ }
;

expr:
  assignexpr                      { __LOG__ }
| expr '+' expr                  { __LOG__ }
| expr '-' expr                 { __LOG__ }
| expr '*' expr                   { __LOG__ }
| expr '/' expr                   { __LOG__ }
| expr '%' expr                   { __LOG__ }
| expr GT expr          { __LOG__ }
| expr GTE expr { __LOG__ }
| expr LT expr             { __LOG__ }
| expr LTE expr    { __LOG__ }
| expr EQ expr                 { __LOG__ }
| expr NEQ expr             { __LOG__ }
| expr AND expr                   { __LOG__ }
| expr OR expr                    { __LOG__ }
| term                            { __LOG__ }
;

term:
  '(' expr ')' { __LOG__ }
| '-' expr %prec UMINUS                 { __LOG__ }
| NOT expr                                { __LOG__ }
| PLUS_PLUS lvalue                        { term__plusplus_lvalue($2); __LOG__ }
| lvalue  PLUS_PLUS                       { term__lvalue_plusplus($1); __LOG__ }
| MINUS_MINUS lvalue                      { term__minusminus_lvalue($2); __LOG__ }
| lvalue MINUS_MINUS                      { term__lvalue_minusminus($1); __LOG__ }
| primary                                 { __LOG__ }
;

assignexpr:
  lvalue { assignexpr__lvalue($1); } '=' expr { __LOG__ }
;

primary:
  lvalue                                     { __LOG__ }
| call                                       { __LOG__ }
| objectdef                                  { __LOG__ }
| '(' funcdef ')' { __LOG__ }
| const                                      { __LOG__ }
;

lvalue:
  ID             { lvalue__id($$, $1); __LOG__ }
| LOCAL ID       { lvalue__local_id($$, $2); __LOG__ }
| COLON_BLOCK ID { lvalue__colonblock_id($$, $2); __LOG__ }
| member         { lvalue__member(); __LOG__ }
;

member:
  lvalue '.' ID                          { __LOG__ }
| lvalue '[' expr ']' { __LOG__ }
| call '.' ID                            { __LOG__ }
| call '[' expr ']'   { __LOG__ }
;

call:
  call '(' elist ')' { __LOG__ }
| lvalue callsuffix                             { __LOG__ }
| '(' funcdef ')'
  '(' elist ')'      { __LOG__ }
;

callsuffix:
  normcall   { __LOG__ }
| methodcall { __LOG__ }
;

normcall:
  '(' elist ')' { __LOG__ }
;

methodcall:
  DDOT ID '(' elist ')' { __LOG__ }
;

expr_list:
  expr                 { __LOG__ }
| expr ',' expr_list { __LOG__ }
;

elist:
            { __LOG__ }
| expr_list { __LOG__ }
;

objectdef:
  '[' elist ']'   { __LOG__ }
| '[' indexed ']' { __LOG__ }
;

indexed:
  indexedelem_list { __LOG__ }
;

indexedelem:
  '{' expr ':' expr '}' { __LOG__ }
;

block:
  '{' { block__leftbrace(); } multi_stmt '}'
  { block__leftbrace_multistmt_rightbrace(); __LOG__ }
| '{' { block__leftbrace(); } '}'
  { block__leftbrace_rightbrace(); __LOG__ }
;

funcdef:
  FUNCTION ID
  { funcdef__function_id($2); } '(' idlist ')'
  { funcdef__function_id_lparen_idlist_rparen(); } block
  { funcdef__function_id_lparen_idlist_rparen(); __LOG__ }
| FUNCTION '(' idlist ')'
  { funcdef__function_lparen_idlist_rparen(); } block
  { funcdef__function_lparen_idlist_rparen_block(); __LOG__ }
;

const:
  INT_CONST      { __LOG__ }
| REAL_CONST     { __LOG__ }
| STRING_LITERAL { __LOG__ }
| NIL            { __LOG__ }
| TRUE           { __LOG__ }
| FALSE          { __LOG__ }
;

cs_ids:
  ID { csids__id($1); __LOG__ }
| ID { csids__id($1); } ',' cs_ids { __LOG__ }
;

idlist:
         { __LOG__ }
| cs_ids { __LOG__ }
;

ifstmt:
  IF '(' expr ')' stmt %prec THEN { __LOG__ }
| IF '(' expr ')' stmt ELSE stmt  { __LOG__ }
;

whilestmt:
  WHILE '(' expr ')'
  {whilestmt__while_lparen_expr_rparen();} stmt
  {whilestmt__while_lparen_expr_rparen_stmt(); __LOG__ }
;

forstmt:
  FOR '(' elist ';' expr ';' elist ')'
  { forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen(); }
  stmt
  { forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen_stmt(); __LOG__ }
;

returnstmt:
  RETURN ';' {returnstmt__return_semicolon(); __LOG__ }
| RETURN expr ';' {returnstmt__return_expr_semicolon(); __LOG__ }
;

indexedelem_list:
  indexedelem                        { __LOG__ }
| indexedelem ',' indexedelem_list { __LOG__ }
;