%code top
{
         // IWYU pragma: no_include <features.h>
         // IWYU pragma: no_include <stdio.h>
         // IWYU pragma: no_include <stdlib.h>
         // IWYU pragma: no_include <string.h>
        #include <string>                             // for basic_string, string
        #include "parser/alpha_trace_logger.hpp"            // for display_trace
        #include "parser/alpha_parser_context.hpp"    // for ParseCtx
        #include "scanner/alpha_scanner_context.hpp"  // for LexerCtx
        #include "alpha_parser_prologue_code.hpp" // THIS MUST STAY in parser's.cpp not parser's .hpp
        using AOP =  Alpha::IOPCode;
}

%code requires
{
        #include "core/alpha_diagnostics.hpp"               // for ErrorTracker
        #include "core/alpha_location.hpp"            // for Location, LocationTracker
        #include "parser/alpha_semantic_manager_.hpp"  // for block__lbrace, funcArgs...
        #include "parser/alpha_semantic_builder.hpp"  // for block__lbrace, funcArgs...
        #include "parser/alpha_parser_context.hpp"    // for ParseCtx
        #include "parser/alpha_symbol_table.hpp"      // for Symbol, SymbolTable
        #include "scanner/alpha_scanner_context.hpp"  // for LexerCtx
}

%define api.prefix {alpha_yy}
%define parse.lac full
%define parse.error verbose    /* Enable verbose error messages */
%define api.location.type {Alpha::Location}
%locations

%parse-param{Alpha::LocationTracker &location_tracker}
%parse-param{Alpha::ErrorTracker &error_tracker}
%parse-param{Alpha::LexerCtx &lexer_ctx}
%parse-param{Alpha::SemanticManager &sm}
%parse-param{Alpha::SemanticBuilder &sb}

%lex-param{Alpha::LocationTracker &location_tracker}
%lex-param{Alpha::ErrorTracker &error_tracker}
%lex-param{Alpha::LexerCtx &lexer_ctx}

// Here I declare the trivial types that can be used in union.
// More complex types are stores in ParseCache of ParseCtx.
// The split is done, because we use Bison C's backend, but mine
// semantic driver is written in C++. Bison's C++ driver is more
// complex and appears to be problematic (erroneous).
%union{
        char *cstring;
        bool const_bool;
        long const_int;
        double const_real;
        const  Alpha::Function *const_function_symbol_ptr;
        Alpha::Expr *expr_ptr;
        Alpha::ExprList *expr_list_ptr;
        Alpha::ExprPair *expr_pair_ptr;
        Alpha::DictList *dict_list_ptr;

        Alpha::Location location;
        Alpha::BlockLocation block_location;
}

/*
 * You can combine a typed %token with a printable name:
 *   %token <field> NAME "display name"
 * Bison will use the quoted string in error messages instead of NAME.
 */
%token <cstring>        STRING "`string-literal`"
%token <cstring>        ID             "`identifier`"
%token <const_int>      INT      "`integer-constant`"
%token <const_real>     REAL     "`real-constant`"

/********************************************************
%type  <expr_ptr> lvalue
%type  <expr_ptr> tableItem
%type  <expr_ptr> member
%type  <expr_ptr> primary
%type  <expr_ptr> assignExpr
%type  <expr_ptr> call
%type  <expr_ptr> term
%type  <expr_ptr> objectDef
%type  <expr_ptr> tableList
%type  <expr_ptr> tableDict
%type  <expr_ptr> const
%type  <expr_ptr> expr

%type  <expr_list_ptr> exprList
%type  <expr_list_ptr> elist
%type  <dict_list_ptr> indexed
%type  <dict_list_ptr> indexedElemList
%type  <expr_pair_ptr> indexedElem

%type  <const_function_symbol_ptr> funcDef
%type  <const_function_symbol_ptr> funcSignature

%type  <location>       blockBegin 
%type  <location>       blockEnd
%type  <block_location> block
*******************************************************/

/* By default Bison uses the bare token names (e.g. IF, METHOD_CALL)
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
%token LOCAL    "keyword `local`"
%token NIL      "keyword `nil`"
%token TRUE     "keyword `true`"
%token FALSE    "keyword `false`"

/* Operator tokens */
%token ASSIGN    "assignment operator ="
%token PLUS      "+" 
%token MINUS     "-"
%token MUL      "*"
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
%token METHOD_CALL    "method-call operator .."
%token COLON         ":"   
%token GLOBAL  "global operator ::"

/* Priorities */
%right ASSIGN

%left OR
%left AND

%nonassoc EQ NEQ
%nonassoc GT GTE LT LTE

%left PLUS MINUS
%left MUL DIV MOD

%right NOT INC DEC UMINUS

%left DOT METHOD_CALL

%left LEFT_BRACKET RIGHT_BRACKET
%left LEFT_PAREN RIGHT_PAREN

%precedence THEN
%precedence ELSE

%start program /* Entry rule. */

%%
program
: // (empty)
| multiStmt
;

multiStmt
: stmt
| stmt
 multiStmt
;

stmt
: expr SEMICOLON
| ifStmt
| whileStmt
| forStmt
| returnStmt SEMICOLON
| loopCtrlStmt SEMICOLON
| block
| funcDef
| SEMICOLON
| error SEMICOLON     { yyerrok; } // Syntax error recovery hook.
| error RIGHT_PAREN   { yyerrok; std::cout << "RPAREN ERRORED" << std::endl; } // Syntax error recovery hook.
| error RIGHT_BRACKET { yyerrok; } // Syntax error recovery hook.
| error RIGHT_BRACE   { yyerrok; } // Syntax error recovery hook.
;

loopCtrlStmt
:  BREAK
| CONTINUE
;

expr[result]
: assignExpr
| expr[left] PLUS  expr[right]
| expr[left] MINUS expr[right]
| expr[left] MUL   expr[right]
| expr[left] DIV   expr[right]
| expr[left] MOD   expr[right]
| expr[left] GT    expr[right]
| expr[left] GTE   expr[right]
| expr[left] LT    expr[right]
| expr[left] LTE   expr[right]
| expr[left] EQ    expr[right]
| expr[left] NEQ   expr[right]
| expr[left] AND 
  saveNextQuadHook expr[right]
| expr[left] OR
  saveNextQuadHook expr[right]
| term
;

saveNextQuadHook:
;

term:
  LEFT_PAREN expr RIGHT_PAREN
| MINUS expr %prec UMINUS
| NOT expr
| INC lvalue
| lvalue INC
| DEC lvalue
| lvalue DEC
| primary
;

assignExpr:
  lvalue ASSIGN expr  
;

primary:
  lvalue
| call
| objectDef
| LEFT_PAREN funcDef RIGHT_PAREN
| const
;


lvalue:
  ID
| LOCAL ID
| GLOBAL ID
| member
;

tableItem:
  lvalue DOT ID
| lvalue LEFT_BRACKET expr RIGHT_BRACKET
| call DOT ID
| call LEFT_BRACKET expr RIGHT_BRACKET
;

member:
  tableItem
;

methodCallId:
  METHOD_CALL ID 
;

//*TODO: ADD normal_call and Method_call and pass needed variable trhoguh a Call struct!! */
call[invocation]:
  call[callable] LEFT_PAREN elist RIGHT_PAREN // <------------------------------------ CHAIN_CALL
| lvalue LEFT_PAREN elist RIGHT_PAREN // <-------------------------------------------- NORMAL_CALL
| lvalue methodCallId LEFT_PAREN elist RIGHT_PAREN // <------------------------------- METHOD_CALL
| LEFT_PAREN funcDef RIGHT_PAREN LEFT_PAREN elist RIGHT_PAREN // <---------------------IIFE_CALL
;

exprList[head]:
  expr 
| expr COMMA exprList[tail]
;

elist:
  /* (empty) */
| exprList
;

tableList:
  LEFT_BRACKET elist RIGHT_BRACKET
;

tableDict:
  LEFT_BRACKET indexed RIGHT_BRACKET
;

objectDef:
  tableList
| tableDict
;

indexed:
  indexedElemList
;

indexedElemList[head]:
  indexedElem
| indexedElem COMMA indexedElemList[tail]
;

indexedElem:
  LEFT_BRACE expr[key] COLON
  expr[value]
  RIGHT_BRACE
;

blockBegin:
  LEFT_BRACE  
;

blockEnd:
  RIGHT_BRACE
;


block:
  blockBegin multiStmt  blockEnd   
| blockBegin blockEnd
;


funcPrefix:
  FUNCTION
| FUNCTION ID
;

funcArgs:
  ID
| ID COMMA funcArgs
;

funcArgList:
  LEFT_PAREN /*Void*/ RIGHT_PAREN
| LEFT_PAREN funcArgs  RIGHT_PAREN
;

funcSignature:
  funcPrefix funcArgList
;

funcDef:
  funcSignature block 
;

const:
  NIL
| TRUE
| FALSE
| INT
| REAL
| STRING
;

ifPrefix
: IF LEFT_PAREN expr RIGHT_PAREN 
;
elsePrefix
: ELSE
;

ifStmt
: ifPrefix stmt %prec THEN
| ifPrefix stmt elsePrefix stmt
;

whileStart:
WHILE
;

whileCondition:
  LEFT_PAREN expr  RIGHT_PAREN
;

whileHeader:
  whileStart
  whileCondition
;

whileStmt:
  whileHeader
  stmt
;

N1:
;
N2:
;
N3:
;

M:
;

forHeader:
  FOR
  LEFT_PAREN
  elist
  SEMICOLON
  M
  expr
  SEMICOLON
  N1
  elist
  RIGHT_PAREN
;

forStmt:
  forHeader
  N2
  stmt
  N3
;


returnStmt:
  RETURN
| RETURN  expr  
;

%%
