%{
        #define INSIDE_BISON_FILE
        #define __LOG__ ((void)0) /* Hook for python script to add logging. */
        #include <string>
        #include <list>
        #include <iostream>
        #include <stdexcept>
        #include "alphaScanner.hpp"
        #include "alphaDefs.hpp"
        #include "errorTracker.hpp"
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
%token IF               ELSE            WHILE           FOR
%token FUNCTION         RETURN          BREAK           CONTINUE
%token AND              NOT             OR              LOCAL
%token TRUE             FALSE           NIL

/* Operator tokens */
%token ASSIGN           PLUS            MINUS           MUL
%token DIV              MOD             EQUAL           GREATER_THAN_OR_EQUAL
%token PLUS_PLUS        MINUS_MINUS     GREATER_THAN    LESS_THAN_OR_EQUAL
%token NOT_EQUAL        LESS_THAN

/* Punctuation tokens */
%token LEFT_BRACE       RIGHT_BRACE     LEFT_BRACKET    RIGHT_BRACKET
%token COLON            COLON_BLOCK     DOT             LEFT_PARENTHESIS
%token DDOT             SEMI_COLON      COMMA           RIGHT_PARENTHESIS

/* Priorities */
%right ASSIGN

%left OR
%left AND

%nonassoc EQUAL NOT_EQUAL
%nonassoc GREATER_THAN GREATER_THAN_OR_EQUAL LESS_THAN LESS_THAN_OR_EQUAL

%left PLUS MINUS
%left MUL DIV MOD

%right NOT PLUS_PLUS MINUS_MINUS

%left DOT DDOT

%left LEFT_BRACKET RIGHT_BRACKET
%left LEFT_PARENTHESIS RIGHT_PARENTHESIS

%precedence THEN
%precedence ELSE

/* Grammar rules: */

/* The idea is that {} is replace by macro LOG_ACTION(rule, case),
 * where rule and case are string filled by python script. */
%%
program:
             { __LOG__; }
| multi_stmt { __LOG__; }
;

stmt:   
  expr SEMI_COLON     { __LOG__ }
| ifstmt              { __LOG__ }
| whilestmt           { __LOG__ }
| forstmt             { __LOG__ }
| returnstmt          { __LOG__ }
| BREAK SEMI_COLON    { stmt__break_semicolon(); __LOG__ }
| CONTINUE SEMI_COLON { stmt__continue_semicolon(); __LOG__ }
| block               { __LOG__ }
| funcdef             { __LOG__ }
| SEMI_COLON          { __LOG__ }
;

multi_stmt:
  stmt            { __LOG__ }
| stmt multi_stmt { __LOG__ }
;

expr:
  assignexpr                      { __LOG__ }
| expr PLUS expr                  { __LOG__ }
| expr MINUS expr                 { __LOG__ }
| expr MUL expr                   { __LOG__ }
| expr DIV expr                   { __LOG__ }
| expr MOD expr                   { __LOG__ }
| expr GREATER_THAN expr          { __LOG__ }
| expr GREATER_THAN_OR_EQUAL expr { __LOG__ }
| expr LESS_THAN expr             { __LOG__ }
| expr LESS_THAN_OR_EQUAL expr    { __LOG__ }
| expr EQUAL expr                 { __LOG__ }
| expr NOT_EQUAL expr             { __LOG__ }
| expr AND expr                   { __LOG__ }
| expr OR expr                    { __LOG__ }
| term                            { __LOG__ }
;

term:
  LEFT_PARENTHESIS expr RIGHT_PARENTHESIS { __LOG__ }
| MINUS expr                              { __LOG__ }
| NOT expr                                { __LOG__ }
| PLUS_PLUS lvalue                        { term__plusplus_lvalue($2); __LOG__ }
| lvalue  PLUS_PLUS                       { term__lvalue_plusplus($1); __LOG__ }
| MINUS_MINUS lvalue                      { term__minusminus_lvalue($2); __LOG__ }
| lvalue MINUS_MINUS                      { term__lvalue_minusminus($1); __LOG__ }
| primary                                 { __LOG__ }
;

assignexpr:
  lvalue { assignexpr__lvalue($1); } ASSIGN expr { __LOG__ }
;

primary:
  lvalue                                     { __LOG__ }
| call                                       { __LOG__ }
| objectdef                                  { __LOG__ }
| LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS { __LOG__ }
| const                                      { __LOG__ }
;

lvalue:
  ID             { lvalue__id($$, $1); __LOG__ }
| LOCAL ID       { lvalue__local_id($$, $2); __LOG__ }
| COLON_BLOCK ID { lvalue__colonblock_id($$, $2); __LOG__ }
| member         { lvalue__member(); __LOG__ }
;

member:
  lvalue DOT ID                          { __LOG__ }
| lvalue LEFT_BRACKET expr RIGHT_BRACKET { __LOG__ }
| call DOT ID                            { __LOG__ }
| call LEFT_BRACKET expr RIGHT_BRACKET   { __LOG__ }
;

call:
  call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS { __LOG__ }
| lvalue callsuffix                             { __LOG__ }
| LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS
  LEFT_PARENTHESIS elist RIGHT_PARENTHESIS      { __LOG__ }
;

callsuffix:
  normcall   { __LOG__ }
| methodcall { __LOG__ }
;

normcall:
  LEFT_PARENTHESIS elist RIGHT_PARENTHESIS { __LOG__ }
;

methodcall:
  DDOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS { __LOG__ }
;

expr_list:
  expr                 { __LOG__ }
| expr COMMA expr_list { __LOG__ }
;

elist:
            { __LOG__ }
| expr_list { __LOG__ }
;

objectdef:
  LEFT_BRACKET elist RIGHT_BRACKET   { __LOG__ }
| LEFT_BRACKET indexed RIGHT_BRACKET { __LOG__ }
;

indexed:
  indexedelem_list { __LOG__ }
;

indexedelem:
  LEFT_BRACE expr COLON expr RIGHT_BRACE { __LOG__ }
;

block:
  LEFT_BRACE { block__leftbrace(); } multi_stmt RIGHT_BRACE
  { block__leftbrace_multistmt_rightbrace(); __LOG__ }
| LEFT_BRACE { block__leftbrace(); } RIGHT_BRACE
  { block_leftbrace_rightbrace(); __LOG__ }
;

funcdef:
  FUNCTION ID
  { funcdef__function_id($2); } LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS
  { funcdef__function_id_leftparenthesis_idlist_rightparenthesis(); } block
  { funcdef__function_id_leftparenthesis_idlist_rightparenthesis(); __LOG__ }
| FUNCTION LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS
  { funcdef__function_leftparenthesis_idlist_rightparenthesis(); } block
  { funcdef__function_leftparenthesis_idlist_rightparenthesis_block(); __LOG__ }
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
| ID { csids__id($1); } COMMA cs_ids { __LOG__ }
;

idlist:
         { __LOG__ }
| cs_ids { __LOG__ }
;

ifstmt:
  IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt %prec THEN { __LOG__ }
| IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt  { __LOG__ }
;

whilestmt:
  WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS 
  {whilestmt__while_leftparenthesis_expr_right_parenthesis();} stmt
  {whilestmt__while_leftparenthesis_expr_right_parenthesis_stmt(); __LOG__ }
;

forstmt:
  FOR LEFT_PARENTHESIS elist SEMI_COLON expr SEMI_COLON elist RIGHT_PARENTHESIS
  { forstmt__for_leftparenthesis_elist_semicolon_expr_semicolon_elist_rightparenthesis(); }
  stmt
  { forstmt__for_leftparenthesis_elist_semicolon_expr_semicolon_elist_rightparenthesis_stmt(); __LOG__ }
;

returnstmt:
  RETURN SEMI_COLON {returnstmt__return_semicolon(); __LOG__ }
| RETURN expr SEMI_COLON {returnstmt__return_expr_semicolon(); __LOG__ }
;

indexedelem_list:
  indexedelem                        { __LOG__ }
| indexedelem COMMA indexedelem_list { __LOG__ }
;