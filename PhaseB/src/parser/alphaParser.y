%{
        #include <string>
        #include <list>
        #include <iostream>
        #include <stdexcept>
        #include "alphaScanner.hpp"
        #include "alphaDefs.hpp"
        #include "errorTracker.hpp"
        #include "semantic_actions.hpp"
        #include "logger.hpp"
        bool isFunctionBlock = false;
        bool lvalueIsMember = false;
        int functionDepthCounter = 0;
%}

%code requires
{
        #include <cstdint>
        #include "symbolTable.hpp"
        extern Alpha::SymbolTable symbolTable;
        typedef struct
        {
                uint32_t line_start;
                uint32_t line_end;
                uint32_t column_start;
                uint32_t column_end;
                uint32_t index_start;
                uint32_t index_end;
        
        }alpha_location_t;

        #define YYLLOC_DEFAULT
        do\
        {\
        }while(0) /* Semi-Colon is placed by bison. */


}

%define api.prefix {alpha_yy}
%define api.location.type {alpha_location_t}
%locations
%define parse.error detailed    /* Enable detailed error messages */

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
%token IF               ELSE
%token WHILE            FOR
%token CONTINUE         BREAK
%token FUNCTION         RETURN
%token NOT              AND             OR      
%token TRUE             FALSE
%token LOCAL            NIL

/* Operator tokens */
%token '='
%token '+'              '-'
%token '*'              '/'             '%'
%token '>'              '<'      
%token GTE              LTE
%token EQ               NEQ      
%token MINUS_MINUS      PLUS_PLUS         

/* Punctuation tokens */
%token  '{'             '}'
%token  '['             ']'
%token  '('             ')'
%token  ';'             ','
%token  '.'             DDOT
%token  ':'             COLON_BLOCK

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

%start program /* Initial rule. */
/* Grammar rules: */

%%
program:

| multi_stmt
;

stmt:   
  expr ';'
| ifstmt
| whilestmt
| forstmt
| returnstmt
| BREAK ';'     { stmt__break_semicolon(); }
| CONTINUE ';'  { stmt__continue_semicolon(); }
| block
| funcdef
| ';'
;

multi_stmt:
  stmt
| stmt multi_stmt
;

expr:
  assignexpr
| expr '+' expr
| expr '-' expr
| expr '*' expr
| expr '/' expr
| expr '%' expr
| expr GT expr
| expr GTE expr
| expr LT expr
| expr LTE expr
| expr EQ expr
| expr NEQ expr
| expr AND expr
| expr OR expr
| term
;

term:
  '(' expr ')'
| '-' expr %prec UMINUS
| NOT expr
| PLUS_PLUS lvalue    { term__plusplus_lvalue($2); }
| lvalue  PLUS_PLUS   { term__lvalue_plusplus($1); }
| MINUS_MINUS lvalue  { term__minusminus_lvalue($2); }
| lvalue MINUS_MINUS  { term__lvalue_minusminus($1); }
| primary                                 
;

assignexpr:
  lvalue { assignexpr__lvalue($1); } '=' expr
;

primary:
  lvalue
| call
| objectdef
| '(' funcdef ')'
| const
;

lvalue:
  ID             { lvalue__id($$, $1); }
| LOCAL ID       { lvalue__local_id($$, $2); }
| COLON_BLOCK ID { lvalue__colonblock_id($$, $2); }
| member         { lvalue__member(); }
;

member:
  lvalue '.' ID
| lvalue '[' expr ']'
| call '.' ID
| call '[' expr ']'
;

call:
  call '(' elist ')'
| lvalue callsuffix
| '(' funcdef ')' '(' elist ')'
;

callsuffix:
  normcall
| methodcall
;

normcall:
  '(' elist ')'
;

methodcall:
  DDOT ID '(' elist ')'
;

expr_list:
  expr
| expr ',' expr_list
;

elist:

| expr_list
;

objectdef:
  '[' elist ']'
| '[' indexed ']'
;

indexed:
  indexedelem_list
;

indexedelem:
  '{' expr ':' expr '}'
;

block:
  '{' { block__leftbrace(); } multi_stmt  '}'  { block__leftbrace_multistmt_rightbrace(); }
| '{' { block__leftbrace(); }             '}'  { block__leftbrace_rightbrace(); }
;

funcdef:
  FUNCTION ID
  { funcdef__function_id($2); } '(' idlist ')'
  { funcdef__function_id_lparen_idlist_rparen(); } block
  { funcdef__function_id_lparen_idlist_rparen_block(); }
| FUNCTION '(' idlist ')'
  { funcdef__function_lparen_idlist_rparen(); } block
  { funcdef__function_lparen_idlist_rparen_block(); }
;

const:
  INT_CONST
| REAL_CONST
| STRING_LITERAL
| NIL
| TRUE
| FALSE
;

cs_ids:
  ID { csids__id($1); }
| ID { csids__id($1); } ',' cs_ids
;

idlist:

| cs_ids
;

ifstmt:
  IF '(' expr ')' stmt %prec THEN
| IF '(' expr ')' stmt ELSE stmt
;

whilestmt:
  WHILE '(' expr ')'
  {whilestmt__while_lparen_expr_rparen();} stmt
  {whilestmt__while_lparen_expr_rparen_stmt(); }
;

forstmt:
  FOR '(' elist ';' expr ';' elist ')'
  { forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen(); }
  stmt
  { forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen_stmt(); }
;

returnstmt:
  RETURN ';' {returnstmt__return_semicolon(); }
| RETURN expr ';' {returnstmt__return_expr_semicolon(); }
;

indexedelem_list:
  indexedelem
| indexedelem ',' indexedelem_list
;

%%
