%code top
{
   // IWYU pragma: no_include <features.h>
   // IWYU pragma: no_include <stdio.h>
   // IWYU pragma: no_include <stdlib.h>
   // IWYU pragma: no_include <string.h>
    #include <string>                             // for basic_string, string
    #include "parser/trace_logger.hpp"      // for display_trace
    #include "parser/parser_context.hpp"    // for ParseCtx
    #include "scanner/scanner_context.hpp"  // for LexerCtx
    #include "parser_prologue_code.hpp"     // THIS MUST STAY in parser's.cpp not parser's .hpp
    using AIOP = Alpha::IOPCode;
}

%code requires
{
    #include "diagnostics/diagnostic_engine.hpp"         // for ErrorTracker
    #include "core/source_location.hpp"     // for Location, LocationTracker
    #include "parser/parser_context.hpp"    // for ParseCtx
    #include "parser/symbol_table.hpp"      // for Symbol, SymbolTable
    #include "scanner/scanner_context.hpp"  // for LexerCtx
    #include "parser/L1_driver/semantic_driver.hpp"
}

%define api.prefix {alpha_yy}
%define parse.lac full
%define parse.error verbose /* Enables verbose error messages */
%define api.location.type {Alpha::SourceLocation}
%locations

%parse-param {Alpha::LocationTracker &location_tracker}
%parse-param {Alpha::DiagnosticEngine &diagnostic_engine}
%parse-param {Alpha::LexerCtx &lexer_ctx}
%parse-param {Alpha::SemanticDriver &sd}

%lex-param {Alpha::LocationTracker &location_tracker}
%lex-param {Alpha::DiagnosticEngine &diagnostic_engine}
%lex-param {Alpha::LexerCtx &lexer_ctx}

// Here I declare the trivial types that can be used in union.
// More complex types are stores in ParseCache of ParseCtx.
// The split is done, because we use Bison C's backend, but mine
// semantic driver is written in C++. Bison's C++ driver is more
// complex and appears to be problematic (erroneous).
%union{
    char *cstring;
    bool const_bool;
    Alpha::AlphaInt const_int;
    Alpha::AlphaFloat const_float;
    const Alpha::Function *const_function_symbol_ptr;
    const Alpha::Expr *const_expr_ptr;
    Alpha::BlockLocation block_location;
}

%type  <const_expr_ptr> const
%type  <const_expr_ptr> primary
%type  <const_expr_ptr> term
%type  <const_expr_ptr> lvalue
%type  <const_expr_ptr> expr
%type  <const_expr_ptr> and_op
%type  <const_expr_ptr> or_op
%type  <const_expr_ptr> assignExpr
/********************************************************
%type  <expr_ptr> tableItem
%type  <expr_ptr> member
%type  <expr_ptr> call
%type  <expr_ptr> objectDef
%type  <expr_ptr> tableList
%type  <expr_ptr> tableDict

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

%token <cstring>        STRING "`string-literal`"
%token <cstring>        ID             "`identifier`"
%token <const_int>      INT      "`integer-constant`"
%token <const_float>    FLOAT     "`float-constant`"

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
program:
  // (empty)
| multiStmt
;

multiStmt:
  stmt
| stmt
 multiStmt
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
| SEMICOLON
| error SEMICOLON     { yyerrok; } // Syntax error recovery hook.
| error RIGHT_PAREN   { yyerrok; std::cout << "RPAREN ERRORED" << std::endl; } // Syntax error recovery hook.
| error RIGHT_BRACKET { yyerrok; } // Syntax error recovery hook.
| error RIGHT_BRACE   { yyerrok; } // Syntax error recovery hook.
;

loopCtrlStmt:
  BREAK
| CONTINUE
;

expr[result]:
  assignExpr { $result = $assignExpr; }
| term       { $result = $term; }
| expr[lhs] PLUS  expr[rhs] { $result = sd.basic_builder.build_arithmetic(AIOP::ADD,          $lhs, $rhs, @result); }
| expr[lhs] MINUS expr[rhs] { $result = sd.basic_builder.build_arithmetic(AIOP::SUB,          $lhs, $rhs, @result); }
| expr[lhs] MUL   expr[rhs] { $result = sd.basic_builder.build_arithmetic(AIOP::MUL,          $lhs, $rhs, @result); }
| expr[lhs] DIV   expr[rhs] { $result = sd.basic_builder.build_arithmetic(AIOP::DIV,          $lhs, $rhs, @result); }
| expr[lhs] MOD   expr[rhs] { $result = sd.basic_builder.build_arithmetic(AIOP::MOD,          $lhs, $rhs, @result); }
| expr[lhs] GT    expr[rhs] { $result = sd.basic_builder.build_relational(AIOP::IF_GREATER,   $lhs, $rhs, @result); }
| expr[lhs] GTE   expr[rhs] { $result = sd.basic_builder.build_relational(AIOP::IF_GREATEREQ, $lhs, $rhs, @result); }
| expr[lhs] LT    expr[rhs] { $result = sd.basic_builder.build_relational(AIOP::IF_LESS,      $lhs, $rhs, @result); }
| expr[lhs] LTE   expr[rhs] { $result = sd.basic_builder.build_relational(AIOP::IF_LESSEQ,    $lhs, $rhs, @result); }
| expr[lhs] EQ    expr[rhs] { $result = sd.basic_builder.build_relational(AIOP::IF_EQ,        $lhs, $rhs, @result); }
| expr[lhs] NEQ   expr[rhs] { $result = sd.basic_builder.build_relational(AIOP::IF_NOTEQ,     $lhs, $rhs, @result); }
| and_op { $result = $and_op; }
| or_op  { $result = $or_op; }
;

and_op:
  expr[lhs]
  AND
  {
    sd.mark_short_circuit_jump_point();
    $lhs = sd.convert_to_bool_expr($lhs);
  }
  expr[rhs]
  {
    $rhs = sd.convert_to_bool_expr($rhs);
    $and_op = sd.basic_builder.build_logical_and($lhs, $rhs, @and_op);
  }
;

or_op:
  expr[lhs]
  OR
  {
    sd.mark_short_circuit_jump_point();
    $lhs = sd.convert_to_bool_expr($lhs);
  }
  expr[rhs]
  {
    $rhs = sd.convert_to_bool_expr($rhs);
    $or_op = sd.basic_builder.build_logical_or($lhs, $rhs, @or_op);
  }
;

term
: primary { $term = $primary; }
| LEFT_PAREN expr RIGHT_PAREN { $term = $expr; }
| MINUS expr %prec UMINUS { $term = sd.basic_builder.build_uminus($expr, @term); }
| NOT expr
| INC lvalue
| lvalue INC
| DEC lvalue
| lvalue DEC
;

assignExpr:
  expr[lhs] ASSIGN expr[rhs] { $assignExpr = sd.assign_builder.build_assignment($lhs, $rhs, @assignExpr); }
;

primary
: const  { $primary = $const; }
| lvalue { $primary = $lvalue; }
| call
| objectDef
| LEFT_PAREN funcDef RIGHT_PAREN
;

lvalue:
  ID { $lvalue = sd.lvalue_resolver.resolve_id($ID, @ID); }
| LOCAL ID
| GLOBAL ID
| tableItem
;

tableItem:
  lvalue DOT ID
| lvalue LEFT_BRACKET expr RIGHT_BRACKET
| call DOT ID
| call LEFT_BRACKET expr RIGHT_BRACKET
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
  LEFT_PAREN /*Void*/  RIGHT_PAREN
| LEFT_PAREN funcArgs  RIGHT_PAREN
;

funcSignature:
  funcPrefix funcArgList
;

funcDef:
  funcSignature block 
;

const
: TRUE      { $const = sd.const_builder.build_true_expr(@TRUE); }
| FALSE     { $const = sd.const_builder.build_false_expr(@FALSE); }
| INT       { $const = sd.const_builder.build_int_expr($INT, @INT); }
| FLOAT     { $const = sd.const_builder.build_float_expr($FLOAT, @FLOAT); }
| STRING    { $const = sd.const_builder.build_string_expr($STRING, @STRING); }
| NIL       { $const = sd.const_builder.build_nil_expr(@NIL); }
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
