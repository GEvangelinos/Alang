%{
        #include <string>
        #include <list>
        #include <iostream>
        #include <stdexcept>
        #include "alpha_scanner.hpp"
        #include "parser/alpha_logger.hpp"
        #include "parser/alpha_semantic_actions.hpp"
        #include "core/alpha_shared_interface.hpp"
        #include "scanner/alpha_scanner_context.hpp"
        #include "parser/alpha_parser_context.hpp"
        extern ALPHA_YYLEX_SIGNATURE;

        #define YYLLOC_DEFAULT(Current, Rhs, N)                                         \
                do                                                                      \
                {                                                                       \
                        if (N)                                                          \
                        {                                                               \
                                (Current).first_index_ = YYRHSLOC(Rhs, 1).first_index_; \
                                (Current).last_index_ = YYRHSLOC(Rhs, N).last_index_;   \
                        }                                                               \
                        else                                                            \
                        {                                                               \
                                (Current).first_index_ = YYRHSLOC(Rhs, 0).last_index_;  \
                                (Current).last_index_ = YYRHSLOC(Rhs, 0).last_index_;   \
                        }                                                               \
                } while (0)

        static void alpha_yyerror(Alpha::LexerCtx &lexer_ctx,
                                Alpha::ParseCtx &parse_ctx,
                                Alpha::SymbolTable &symbol_table,
                                Alpha::ErrorTracker &error_tracker,
                                const std::string &error_message)
        {
        #ifdef DEBUG_MODE
        /* TODO, fill to be able to print errors: */
                (void)lexer_ctx;
                (void)parse_ctx;
                (void)symbol_table;
                (void)error_tracker;
                (void)error_message;
        #endif // DEBUG_MODE
}
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

%define api.location.type {Alpha::CodeLocation}
%locations

%parse-param{Alpha::LexerCtx &lexer_ctx}
%parse-param{Alpha::ParseCtx &parse_ctx}
%parse-param{Alpha::SymbolTable &symbol_table}
%parse-param{Alpha::ErrorTracker &error_tracker}

%lex-param{Alpha::LexerCtx &lexer_ctx}
%lex-param{Alpha::ErrorTracker &error_tracker}

%union{
        char *union_string_literal;
        char *union_id;
        long union_int_const;
        double union_real_const;
        const Alpha::Symbol *union_lvalue;
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

multi_stmt:
  stmt
| stmt multi_stmt
;

stmt:   
  expr ';'
| if_stmt
| while_stmt
| for_stmt
| return_stmt
| loopcontrol_stmt ';'
| block
| funcdef
| ';'
;

loopcontrol_stmt:
  BREAK    { loopcontrol_stmt__break(parse_ctx, @1, error_tracker); }
| CONTINUE { loopcontrol_stmt__continue(parse_ctx, @1, error_tracker); }


expr:
  assign_expr
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
| PLUS_PLUS lvalue
| lvalue  PLUS_PLUS
| MINUS_MINUS lvalue
| lvalue MINUS_MINUS
| primary                                 
;

assign_expr:
  lvalue '=' expr
;

primary:
  lvalue
| call
| objectdef
| '(' funcdef ')'
| const
;

lvalue:
  ID
| LOCAL ID
| COLON_BLOCK ID
| member
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
  '{' multi_stmt  '}'
| '{'             '}'
;

funcdef:
  FUNCTION ID '(' idlist ')' block
| FUNCTION    '(' idlist ')' block
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
  ID
| ID ',' cs_ids
;

idlist:

| cs_ids
;

if_stmt:
  IF '(' expr ')' stmt %prec THEN
| IF '(' expr ')' stmt ELSE stmt
;

while_stmt:
  WHILE '(' expr ')' stmt
;

for_stmt:
  FOR '(' elist ';' expr ';' elist ')' stmt
;

funcctrl_stmt: //OK
  RETURN { funcctrl_stmt__return(parse_ctx, @1, error_tracker); }
;

return_stmt: //OK
  funcctrl_stmt ';'
| funcctrl_stmt expr ';'
;

indexedelem_list:
  indexedelem
| indexedelem ',' indexedelem_list
;

%%
