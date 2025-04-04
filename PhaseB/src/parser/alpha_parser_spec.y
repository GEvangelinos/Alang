%{
        #include <string>
        #include <list>
        #include <iostream>
        #include <stdexcept>
        #include "alpha_scanner.hpp"
        #include "parser/alpha_logger.hpp"
        #include "parser/alpha_semantic_actions.hpp"
        #include "parser/_internal_alpha_parser.hpp"
%}

%code requires
{
        #include "scanner/alpha_scanner_context.hpp"
        #include "parser/alpha_parser_context.hpp"
        #include "parser/alpha_symbol_table.hpp"
        #include "core/alpha_location.hpp"
        #include "core/alpha_error_tracker.hpp"
}

%define api.prefix {alpha_yy}
%define parse.error detailed    /* Enable detailed error messages */

%define api.location.type {Alpha::Location}
%locations

%parse-param{Alpha::ScannerContext &scanner_context}
%parse-param{Alpha::ParserContext &parser_context}
%parse-param{Alpha::SymbolTable &symbol_table}
%parse-param{Alpha::ErrorTracker &error_tracker}

%lex-param{Alpha::ScannerContext &scanner_context}
%lex-param{Alpha::ErrorTracker &error_tracker}

%union{
        char *union_string_literal;
        char *union_id;
        long union_int_const;
        double union_real_const;
        const Alpha::SymbolTableEntry *union_lvalue;
}

%token <union_string_literal> STRING_LITERAL
%token <union_id>ID
%token <union_int_const> INT_CONST
%token <union_real_const> REAL_CONST
%type <union_lvalue> lvalue

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
| BREAK ';'     { stmt__break_semicolon(parser_context, error_tracker, @1); }
| CONTINUE ';'  { stmt__continue_semicolon(parser_context, error_tracker, @1); }
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
| PLUS_PLUS lvalue    { term__plusplus_lvalue(error_tracker, $2, @$); }
| lvalue  PLUS_PLUS   { term__lvalue_plusplus(error_tracker, $1, @$); }
| MINUS_MINUS lvalue  { term__minusminus_lvalue(error_tracker, $2, @$); }
| lvalue MINUS_MINUS  { term__lvalue_minusminus(error_tracker, $1, @$); }
| primary                                 
;

assignexpr:
  lvalue { assignexpr__lvalue(parser_context, error_tracker, $1, @1); } '=' expr
;

primary:
  lvalue
| call
| objectdef
| '(' funcdef ')'
| const
;

lvalue:
  ID             { lvalue__id(symbol_table, error_tracker, &$$, $1, @1); }
| LOCAL ID       { lvalue__local_id(symbol_table, error_tracker, &$$, $2, @$, @2); }
| COLON_BLOCK ID { lvalue__colonblock_id(symbol_table, error_tracker, &$$, $2, @$); }
| member         { lvalue__member(parser_context); }
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
  '{' { block__leftbrace(symbol_table); } multi_stmt  '}'  { block__leftbrace_multistmt_rightbrace(symbol_table); }
| '{' { block__leftbrace(symbol_table); }             '}'  { block__leftbrace_rightbrace(symbol_table); }
;

funcdef:
  FUNCTION ID '(' idlist ')'
  { funcdef__function_id_lparen_idlist_rparen(symbol_table, parser_context, error_tracker, $2, @$); } block
  { funcdef__function_id_lparen_idlist_rparen_block(parser_context); }
| FUNCTION '(' idlist ')'
  { funcdef__function_lparen_idlist_rparen(symbol_table, parser_context, @$); } block
  { funcdef__function_lparen_idlist_rparen_block(parser_context); }
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
  ID { csids__id(parser_context, $1); }
| ID { csids__id(parser_context, $1); } ',' cs_ids
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
  {whilestmt__while_lparen_expr_rparen(parser_context);} stmt
  {whilestmt__while_lparen_expr_rparen_stmt(parser_context); }
;

forstmt:
  FOR '(' elist ';' expr ';' elist ')'
  { forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen(parser_context); }
  stmt
  { forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen_stmt(parser_context); }
;

returnstmt:
  RETURN ';' {returnstmt__return_semicolon(parser_context, error_tracker,@1); }
| RETURN expr ';' {returnstmt__return_expr_semicolon(parser_context, error_tracker, @1); }
;

indexedelem_list:
  indexedelem
| indexedelem ',' indexedelem_list
;

%%
