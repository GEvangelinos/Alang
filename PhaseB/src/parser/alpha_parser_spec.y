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
        // TODO: say in your report for  the progect that ';' is not just a plain syntax requirement.
        // but also the parser's sync point, anything goes wrong, (syntax error) parser can continue parsing gracefully after ';'

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
                                Alpha::LocationTracker &lt,
                                const std::string &error_message)
        {
                extern Location alpha_yylloc;
                error_tracker.report_syntax_error(
                        error_message, Location(alpha_yylloc.first_index_, alpha_yylloc.last_index_));
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

%define api.location.type {Alpha::Location}
%locations

%parse-param{Alpha::LexerCtx &lexer_ctx}
%parse-param{Alpha::ParseCtx &parse_ctx}
%parse-param{Alpha::SymbolTable &symbol_table}
%parse-param{Alpha::ErrorTracker &error_tracker}
%parse-param{Alpha::LocationTracker &location_tracker}

%lex-param{Alpha::LexerCtx &lexer_ctx}
%lex-param{Alpha::ErrorTracker &error_tracker}
%lex-param{Alpha::LocationTracker &location_tracker}

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
%token '+'      '-'
%token '*'      '/'     '%'
%token '>'      '<'      
%token GTE      LTE
%token EQ       NEQ      
%token DEC      INC        

/* Punctuation tokens */
%token  '{'             '}'
%token  '['             ']'
%token  '('             ')'
%token  ';'             ','
%token  '.'             METHOD_CALL
%token  ':'             GLOBAL

/* Priorities */
%right '='

%left OR
%left AND

%nonassoc EQ NEQ
%nonassoc GT GTE LT LTE

%left '+' '-'
%left '*' '/' '%'

%right NOT INC DEC UMINUS

%left '.' METHOD_CALL

%left '[' ']'
%left '(' ')'

%precedence THEN
%precedence ELSE

%start program /* Initial rule. */
/* Grammar rules: */

%%
program:
  // (empty)
| multiStmt
;

multiStmt:
  stmt
| stmt multiStmt
;

stmt:   
  expr ';'
| ifStmt
| whileStmt
| forStmt
| returnStmt ';'
| loopCtrlStmt ';'
| block
| funcDef
| ';'
| error ';'     { yyerrok; } // Syntax error recovery hook.
| error ')'     { yyerrok; } // Syntax error recovery hook.
| error ']'     { yyerrok; } // Syntax error recovery hook.
| error '}'     { yyerrok; } // Syntax error recovery hook.
;

loopCtrlStmt:
  BREAK    { loopCtrlStmt__break(parse_ctx, @1, error_tracker); }
| CONTINUE { loopCtrlStmt__continue(parse_ctx, @1, error_tracker); }
;

expr:
  assignExpr
| expr '+' expr
| expr '-' expr
| expr '*' expr
| expr '/' expr
| expr '%' expr
| expr GT  expr
| expr GTE expr
| expr LT  expr
| expr LTE expr
| expr EQ  expr
| expr NEQ expr
| expr AND expr
| expr OR  expr
| term
;

term:
  '(' expr ')'
| '-' expr %prec UMINUS
| NOT expr
| INC lvalue { term__inc_lvalue($2, @$, error_tracker); }
| lvalue INC { term__lvalue_inc($1, @$, error_tracker); }
| DEC lvalue { term__dec_lvalue($2, @$, error_tracker); }
| lvalue DEC { term__lvalue_dec($1, @$, error_tracker); }
| primary
;

assignExpr:
  lvalue '=' expr { assignExpr__lvalue_assign_expr($1, @2, error_tracker); }
;

primary:
  lvalue
| call
| objectDef
| '(' funcDef ')'
| const
;

lvalue:
  ID { lvalue__id(symbol_table, parse_ctx, $1, @1, &$$, error_tracker); }
| LOCAL ID { lvalue__local_id(symbol_table, parse_ctx, $2, @2, &$$, error_tracker); } 
| GLOBAL ID { lvalue__global_id(symbol_table, $2, @2, &$$, error_tracker); }
| member { lvalue__member(&$$); }
;

member:
  lvalue '.' ID
| lvalue '[' expr ']'
| call '.' ID
| call '[' expr ']'
;

call:
  call '(' elist ')'
| lvalue callSuffix
| '(' funcDef ')' '(' elist ')'
;

callSuffix:
  normalCall
| methodCall
;

normalCall:
  '(' elist ')'
;

methodCall:
  METHOD_CALL ID '(' elist ')'
;

exprList:
  expr
| expr ',' exprList
;

elist:
  // (empty)
| exprList
;

objectDef:
  '[' elist ']'
| '[' indexed ']'
;

indexed:
  indexedElemList
;

indexedElem:
  '{' expr ':' expr '}'
;

block:
  '{' { block__lbrace(parse_ctx); } multiStmt  '}' { block__lbrace_multiStmt_rbrace(symbol_table ,parse_ctx); }
| '{' { block__lbrace(parse_ctx); }            '}' { block__lbrace_rbrace(symbol_table, parse_ctx); }
;

funcDef:
  FUNCTION ID 
  { funcdef__function_id(parse_ctx, $2, @2); }
  '(' funcArgList ')'
  { funcDef__function_id_lparen_funcArgList_rparen(symbol_table, parse_ctx, error_tracker); }
  block
  { funcDef__function_id_lparen_funcArgList_rparen_block(parse_ctx); }
| FUNCTION '(' funcArgList ')'
  { funcDef__function_lparen_funcArgList_rparen(symbol_table, parse_ctx, @1, error_tracker); }
  block
  { funcDef__function_lparen_funcArgList_rparen_block(parse_ctx); }
;

const:
  INT_CONST
| REAL_CONST
| STRING_LITERAL
| NIL
| TRUE
| FALSE
;

funcArgs:
  ID { funcArgs__id(parse_ctx, $1, @1); }
| ID { funcArgs__id(parse_ctx, $1, @1); } ',' funcArgs
;

funcArgList:
  // (empty)
| funcArgs
;

ifStmt:
  IF '(' expr ')' stmt %prec THEN
| IF '(' expr ')' stmt ELSE stmt
;

whileHeader:
  WHILE '(' expr ')' 
;

whileStmt:
  whileHeader
  { whileStmt__whileHeader(parse_ctx); }
  stmt
  { whileStmt__whileHeader_stmt(parse_ctx); }
;

forHeader:
  FOR '(' elist ';' expr ';' elist ')'
;

forStmt:
  forHeader
  { forStmt__forHeader(parse_ctx); } 
  stmt
  { forStmt__forHeader_stmt(parse_ctx); }
;

funcCtrlStmt: //OK
  RETURN { funcCtrlStmt__return(parse_ctx, @1, error_tracker); }
;

returnStmt: //OK
  funcCtrlStmt
| funcCtrlStmt expr
;

indexedElemList:
  indexedElem
| indexedElem ',' indexedElemList
;

%%
