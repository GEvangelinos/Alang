%code top
{
         // IWYU pragma: no_include <features.h>
         // IWYU pragma: no_include <stdio.h>
         // IWYU pragma: no_include <stdlib.h>
         // IWYU pragma: no_include <string.h>
        #include <string>                             // for basic_string, string
        #include "parser/alpha_logger.hpp"            // for display_trace
        #include "parser/alpha_parser_context.hpp"    // for ParseCtx
        #include "parser/alpha_semantic_actions.hpp"  // for block__lbrace, funcArgs...
        #include "scanner/alpha_scanner_context.hpp"  // for LexerCtx
        #include "alpha_parser_prologue_code.hpp"
}

%code requires
{
        #include "core/alpha_error.hpp"               // for ErrorTracker
        #include "core/alpha_location.hpp"            // for Location, LocationTracker
        #include "parser/alpha_parser_context.hpp"    // for ParseCtx
        #include "parser/alpha_symbol_table.hpp"      // for Symbol, SymbolTable
        #include "scanner/alpha_scanner_context.hpp"  // for LexerCtx
}

%define api.prefix {alpha_yy}
%define parse.lac full
%define parse.error verbose    /* Enable verbose error messages */

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

/*
 * You can combine a typed %token with a printable name:
 *   %token <field> NAME "display name"
 * Bison will use the quoted string in error messages instead of NAME.
 */
%token <union_string_literal> STRING_LITERAL "`string-literal`"
%token <union_id>             ID             "`identifier`"
%token <union_int_const>      INT_CONST      "`integer-constant`"
%token <union_real_const>     REAL_CONST     "`real-constant`"
%type <union_lvalue> lvalue

/*
 * By default Bison uses the bare token names (e.g. IF, GLOBAL)
 * in its syntax‐error messages.  If you follow a %token with
 * a quoted string, Bison will use that string instead as the
 * token’s “display name.”  That way you get messages like:
 *
 *     syntax error, unexpected "keyword `if`"
 *     syntax error, unexpected "global operator `::`"
 *
 * instead of
 *
 *     syntax error, unexpected IF
 *     syntax error, unexpected GLOBAL
 */

/* Keyword tokens */
%token IF       "keyword `if`"
%token ELSE     "keyword `else`"
%token WHILE    "keyword `while`"        
%token FOR      "keyword `for`"
%token CONTINUE "keyword `continue`"        
%token BREAK    "keyword `break`"
%token FUNCTION "keyword `function`"     
%token RETURN   "keyword `return`"
%token NOT      "keyword `not`"
%token AND      "keyword `and`"
%token OR       "keyword `or`"
%token TRUE     "keyword `true`"
%token FALSE    "keyword `false`"
%token LOCAL    "keyword `local`"
%token NIL      "keyword `nil`"

/* Operator tokens */
%token '='      "assignment operator `=`"
%token '+'      "addition operator `+`" 
%token '-'      "subtraction operator `-`"
%token '*'      "multiplication operator `*`"
%token '/'      "division operator `/`"
%token '%'      "modulo operator `%`"
%token '>'      "greater-than operator `>`"
%token '<'      "less-than operator `<`"
%token GTE      "greater-than-or-equal operator `>=`"
%token LTE      "less-than-or-equal operator `<=`"
%token EQ       "equal operator `==`"
%token NEQ      "not-equal operator `!=`"     
%token DEC      "decrement operator `--`"
%token INC      "increment operator `++`"

/* Punctuation tokens */
%token '{'          "`{`"
%token '}'          "`}`"
%token '['          "`[`"
%token ']'          "`]`"
%token '('          "`(`" 
%token ')'          "`)`"
%token ';'          "`;`"   
%token ','          "`,`"
%token '.'          "`.`"   
%token METHOD_CALL  "method-call operator `..`"
%token ':'          "`:`"   
%token GLOBAL       "global operator `::`"

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

%start program /* Entry rule. */

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
  BREAK    { loopCtrlStmt__break(parse_ctx, @BREAK, error_tracker); }
| CONTINUE { loopCtrlStmt__continue(parse_ctx, @CONTINUE, error_tracker); }
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
| INC lvalue { term__inc_lvalue($lvalue, @term, error_tracker); }
| lvalue INC { term__lvalue_inc($lvalue, @term, error_tracker); }
| DEC lvalue { term__dec_lvalue($lvalue, @term, error_tracker); }
| lvalue DEC { term__lvalue_dec($lvalue, @term, error_tracker); }
| primary
;

assignExpr:
  lvalue '='[assign_op] expr { assignExpr__lvalue_assign_expr($lvalue, @assign_op, error_tracker); }
;

primary:
  lvalue
| call
| objectDef
| '(' funcDef ')'
| const
;


lvalue:
  ID { lvalue__id(symbol_table, parse_ctx, $ID, @ID, $lvalue, error_tracker); }
| LOCAL ID { lvalue__local_id(symbol_table, parse_ctx, $ID, @ID, $lvalue, error_tracker); } 
| GLOBAL ID { lvalue__global_id(symbol_table, $ID, @ID, $lvalue, error_tracker); }
| member { lvalue__member($lvalue); }
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

blockOpen:
  '{' { blockOpen__lbrace(parse_ctx); }
;

blockClose:
  '}' { blockClose__rbrace(symbol_table, parse_ctx); }
;

block:
  blockOpen multiStmt  blockClose
| blockOpen blockClose
;


funcPrefix:
  FUNCTION      { funcPrefix__function(parse_ctx, @FUNCTION); }
| FUNCTION ID   { funcPrefix__function_id(parse_ctx, $ID, @ID); }
;

funcArgs:
  ID { funcArgs__id(parse_ctx, $ID, @ID); }
| ID { funcArgs__id(parse_ctx, $ID, @ID); } ',' funcArgs
;

funcArgList:
  '(' /*Void*/ ')'
| '(' funcArgs ')'
;

funcSignature:
  funcPrefix  funcArgList
  { funcSignature__funcPrefix_funcArgList(symbol_table, parse_ctx, error_tracker); }
;

funcDef:
  funcSignature block { funcDef__funcSignature_block(parse_ctx); }
;

const:
  INT_CONST
| REAL_CONST
| STRING_LITERAL { const__stringliteral($STRING_LITERAL);}
| NIL
| TRUE
| FALSE
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
  RETURN { funcCtrlStmt__return(parse_ctx, @RETURN, error_tracker); }
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
