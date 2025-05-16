%code top
{
         // IWYU pragma: no_include <features.h>
         // IWYU pragma: no_include <stdio.h>
         // IWYU pragma: no_include <stdlib.h>
         // IWYU pragma: no_include <string.h>
        #include <string>                             // for basic_string, string
        #include "parser/alpha_trace_logger.hpp"            // for display_trace
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

// Here I declare the trivial types that can be used in union.
// More complex types are stores in ParseCache of ParseCtx.
// The split is done, because we use Bison C's backend, but mine
// semantic driver is written in C++. Bison's C++ driver is more
// complex and appears to be problematic (erroneous).
%union{
        char *cstring;
        long const_int;
        double const_real;
        const Alpha::Symbol *symbol_ptr;
        Alpha::Location location;
        Alpha::BlockLocation block_location;
}

/*
 * You can combine a typed %token with a printable name:
 *   %token <field> NAME "display name"
 * Bison will use the quoted string in error messages instead of NAME.
 */
%token <cstring>        STRING_LITERAL "`string-literal`"
%token <cstring>        ID             "`identifier`"
%token <const_int>      INT_CONST      "`integer-constant`"
%token <const_real>     REAL_CONST     "`real-constant`"
%type  <symbol_ptr>     lvalue
%type  <location>       blockOpen 
%type  <location>       blockClose
%type  <block_location> block

/* By default Bison uses the bare token names (e.g. IF, GLOBAL)
 * in its syntax‐error messages.  If you follow a %token with
 * a quoted string, Bison will use that string instead as the
 * token’s “display name.”  That way you get messages like:
 *
 *     syntax error, unexpected "keyword if"
 *     syntax error, unexpected "global operator ::"
 *
 * instead of
 *
 *     syntax error, unexpected IF
 *     syntax error, unexpected DOUBLE_COLON
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
%token ASSIGN    "assignment operator ="
%token PLUS      "+" 
%token MINUS     "-"
%token MULT      "*"
%token DIV       "/"
%token MOD       "%"
%token LT        ">"
%token GT        "<"
%token GTE       ">="
%token LTE       "<="
%token EQ        "=="
%token NEQ       "!="     
%token DEC       "decrement operator `--`"
%token INC       "increment operator `++`"

/* Punctuation tokens */
%token LEFT_BRACE    "{"
%token RIGHT_BRACE   "}"
%token LEFT_BRACKET  "["
%token RIGHT_BRACKET "]"
%token LEFT_PAREN    "(" 
%token RIGHT_PAREN   ")"
%token SEMICOLON     ";"   
%token COMMA         ","
%token DOT           "."   
%token DOUBLE_DOT    "method-call operator .."
%token COLON         ":"   
%token DOUBLE_COLON  "global operator ::"

/* Priorities */
%right ASSIGN

%left OR
%left AND

%nonassoc EQ NEQ
%nonassoc GT GTE LT LTE

%left PLUS MINUS
%left MULT DIV MOD

%right NOT INC DEC UMINUS

%left DOT DOUBLE_DOT

%left LEFT_BRACKET RIGHT_BRACKET
%left LEFT_PAREN RIGHT_PAREN

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
  expr SEMICOLON
| ifStmt
| whileStmt
| forStmt
| returnStmt SEMICOLON
| loopCtrlStmt SEMICOLON
| block
| funcDef
|SEMICOLON
| error SEMICOLON     { yyerrok; } // Syntax error recovery hook.
| error RIGHT_PAREN   { yyerrok; } // Syntax error recovery hook.
| error RIGHT_BRACKET { yyerrok; } // Syntax error recovery hook.
| error RIGHT_BRACE   { yyerrok; } // Syntax error recovery hook.
;

loopCtrlStmt:
  BREAK    { loopCtrlStmt__break(parse_ctx, @BREAK, error_tracker); }
| CONTINUE { loopCtrlStmt__continue(parse_ctx, @CONTINUE, error_tracker); }
;

expr:
  assignExpr
| expr PLUS  expr
| expr MINUS expr
| expr MULT  expr
| expr DIV   expr
| expr MOD   expr
| expr GT    expr
| expr GTE   expr
| expr LT    expr
| expr LTE   expr
| expr EQ    expr
| expr NEQ   expr
| expr AND   expr
| expr OR    expr
| term
;

term:
  LEFT_PAREN expr RIGHT_PAREN
| MINUS expr %prec UMINUS
| NOT expr
| INC lvalue { term__inc_lvalue($lvalue, @term, error_tracker); }
| lvalue INC { term__lvalue_inc($lvalue, @term, error_tracker); }
| DEC lvalue { term__dec_lvalue($lvalue, @term, error_tracker); }
| lvalue DEC { term__lvalue_dec($lvalue, @term, error_tracker); }
| primary
;

assignExpr:
  lvalue ASSIGN expr { assignExpr__lvalue_assign_expr($lvalue, @ASSIGN, error_tracker); }
;

primary:
  lvalue
| call
| objectDef
| LEFT_PAREN funcDef RIGHT_PAREN
| const
;


lvalue:
  ID { lvalue__id(symbol_table, parse_ctx, $ID, @ID, $lvalue, error_tracker); }
| LOCAL ID { lvalue__local_id(symbol_table, parse_ctx, $ID, @ID, $lvalue, error_tracker); } 
| DOUBLE_COLON ID { lvalue__global_id(symbol_table, $ID, @ID, $lvalue, error_tracker); }
| member { lvalue__member($lvalue); }
;

member:
  lvalue DOT ID
| lvalue LEFT_BRACKET expr RIGHT_BRACKET
| call DOT ID
| call LEFT_BRACKET expr RIGHT_BRACKET
;

call:
  call LEFT_PAREN elist RIGHT_PAREN
| lvalue callSuffix
| LEFT_PAREN funcDef RIGHT_PAREN LEFT_PAREN elist RIGHT_PAREN
;

callSuffix:
  normalCall
| methodCall
;

normalCall:
  LEFT_PAREN elist RIGHT_PAREN
;

methodCall:
  DOUBLE_DOT ID LEFT_PAREN elist RIGHT_PAREN
;

exprList:
  expr
| expr COMMA exprList
;

elist:
  // (empty)
| exprList
;

objectDef:
  LEFT_BRACKET elist RIGHT_BRACKET
| LEFT_BRACKET indexed RIGHT_BRACKET
;

indexed:
  indexedElemList
;

indexedElem:
  LEFT_BRACE expr COLON expr RIGHT_BRACE
;

blockOpen:
  LEFT_BRACE
  { 
    blockOpen__lbrace(parse_ctx);
    $blockOpen = @LEFT_BRACE;
  }
;

blockClose:
  RIGHT_BRACE
  { 
    blockClose__rbrace(symbol_table, parse_ctx);
    $blockClose = @RIGHT_BRACE;
  }
;

block:
  blockOpen multiStmt  blockClose 
  { $block = BlockLocation{.begin = $blockOpen, .end = $blockClose}; }
| blockOpen blockClose
  { $block = BlockLocation{.begin = $blockOpen, .end = $blockClose}; }
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
  LEFT_PAREN /*Void*/ RIGHT_PAREN
| LEFT_PAREN funcArgs RIGHT_PAREN
;

funcSignature:
  funcPrefix  funcArgList
  { funcSignature__funcPrefix_funcArgList(symbol_table, parse_ctx, error_tracker); }
;

funcDef:
  funcSignature block { funcDef__funcSignature_block(parse_ctx, $block); }
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
  IF LEFT_PAREN expr RIGHT_PAREN stmt %prec THEN
| IF LEFT_PAREN expr RIGHT_PAREN stmt ELSE stmt
;

whileHeader:
  WHILE LEFT_PAREN expr RIGHT_PAREN 
;

whileStmt:
  whileHeader
  { whileStmt__whileHeader(parse_ctx); }
  stmt
  { whileStmt__whileHeader_stmt(parse_ctx); }
;

forHeader:
  FOR LEFT_PAREN elist SEMICOLON expr SEMICOLON elist RIGHT_PAREN
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
| indexedElem COMMA indexedElemList
;

%%
