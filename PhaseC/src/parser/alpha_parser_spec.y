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
        #include "alpha_parser_prologue_code.hpp"
}

%code requires
{
        #include "core/alpha_error.hpp"               // for ErrorTracker
        #include "core/alpha_location.hpp"            // for Location, LocationTracker
        #include "parser/alpha_semantic_manager.hpp"  // for block__lbrace, funcArgs...
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
        std::vector<Alpha::Expr *> *expr_list_ptr; // Keep vector instead of list for better cache locality.
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

%type  <expr_ptr> lvalue
%type  <expr_ptr> tableItem
%type  <expr_ptr> member
%type  <expr_ptr> primary
%type  <expr_ptr> assignExpr
%type  <expr_ptr> call
%type  <expr_ptr> term
%type  <expr_ptr> objectDef
%type  <expr_ptr> const
%type  <expr_ptr> expr

%type  <expr_list_ptr> exprList
%type  <expr_list_ptr> elist

%type  <const_function_symbol_ptr> funcDef
%type  <const_function_symbol_ptr> funcSignature


%type  <location>       blockBegin 
%type  <location>       blockEnd
%type  <block_location> block

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
%left MULT DIV MOD

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
| SEMICOLON
| error SEMICOLON     { yyerrok; } // Syntax error recovery hook.
| error RIGHT_PAREN   { yyerrok; } // Syntax error recovery hook.
| error RIGHT_BRACKET { yyerrok; } // Syntax error recovery hook.
| error RIGHT_BRACE   { yyerrok; } // Syntax error recovery hook.
;

loopCtrlStmt:
  BREAK    { sm.loopCtrlStmt__break(@BREAK); }
| CONTINUE { sm.loopCtrlStmt__continue(@CONTINUE); }
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
| term { $expr = $term; }
;

term:
  LEFT_PAREN expr RIGHT_PAREN
| MINUS expr %prec UMINUS
| NOT expr
| INC lvalue { sm.term__inc_lvalue($lvalue, @term); }
| lvalue INC { sm.term__lvalue_inc($lvalue, @term); }
| DEC lvalue { sm.term__dec_lvalue($lvalue, @term); }
| lvalue DEC { sm.term__lvalue_dec($lvalue, @term); }
| primary { $term = $primary; }
;

assignExpr:
  lvalue ASSIGN expr  
  { $assignExpr = sb.resolve_assign_expr($lvalue,$expr, @ASSIGN); }
;

primary:
  lvalue { $primary = sb.resolve_lvalue_to_primary($lvalue); }
| call
| objectDef
| LEFT_PAREN funcDef RIGHT_PAREN
| const  { $primary = $const; }
;


lvalue:
  ID        { sm.lvalue__id($lvalue, $ID, @ID); }
| LOCAL ID  { sm.lvalue__local_id($lvalue, $ID, @ID); } 
| GLOBAL ID { sm.lvalue__global_id($lvalue, $ID, @ID); }
| member { $lvalue = $member; }
;

tableItem:
  lvalue DOT ID
  { $tableItem = sb.make_table_item($lvalue, $ID, @tableItem ,@ID); } 
| lvalue LEFT_BRACKET expr RIGHT_BRACKET 
  { $tableItem = sb.make_table_item($lvalue, $expr, @tableItem); } 
;

member:
  tableItem { $member = $tableItem; }
| call DOT ID
| call LEFT_BRACKET expr RIGHT_BRACKET
;

//*TODO: ADD normal_call and Method_call and pass needed variable trhoguh a Call struct!! */
call[invocation]:
  call[callable] LEFT_PAREN elist RIGHT_PAREN
  { $invocation = sb.make_call($callable, $elist, @invocation); }
| lvalue LEFT_PAREN elist RIGHT_PAREN // NORMAL_CALL
  { $invocation = sb.make_normal_call($lvalue, $elist, @invocation); }
| lvalue METHOD_CALL ID LEFT_PAREN elist RIGHT_PAREN // METHOD_CALL
  { $invocation = sb.make_method_call($lvalue, $ID, $elist, @invocation, @ID); }
| LEFT_PAREN funcDef RIGHT_PAREN LEFT_PAREN elist RIGHT_PAREN
  { $invocation = sb.make_iife_call($funcDef, $elist, @invocation); }
;

exprList[head]:
  expr
  { 
    sm.update_expr_location($expr, @expr);
    $head = sb.make_empty_expr_list();
    $head = sb.extend_expr_list($expr, $exprList);
  }
| expr COMMA exprList[tail] 
  {
    sm.update_expr_location($expr, @expr);
    $head = sb.extend_expr_list($expr, $tail);
  }
;

elist:
  /* (empty) */ { $elist = sb.make_empty_expr_list(); }
| exprList      { $elist = $exprList; }
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

blockBegin:
  LEFT_BRACE  { blockBegin__lbrace(parse_ctx); $blockBegin = @LEFT_BRACE; }
;

blockEnd:
  RIGHT_BRACE { blockEnd__rbrace(symbol_table, parse_ctx); $blockEnd = @RIGHT_BRACE; }
;

block:
  blockBegin multiStmt  blockEnd   { $block = ASF::make_block_location($blockBegin, $blockEnd); }
| blockBegin blockEnd              { $block = ASF::make_block_location($blockBegin, $blockEnd); }
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
  { funcSignature__funcPrefix_funcArgList(symbol_table, parse_ctx, error_tracker, $funcSignature); }
;

funcDef:
  funcSignature block 
  { 
    // TODO to much inderection remove funcSignature rule and MERGE... 
    funcDef__funcSignature_block(parse_ctx, $block); 
    $funcDef = $funcSignature;
  }
;

const:
  NIL    { $const = ASF::make_const_nil(parse_ctx, @NIL); }
| TRUE   { $const = ASF::make_const_true(parse_ctx, @TRUE); }
| FALSE  { $const = ASF::make_const_false(parse_ctx, @FALSE); }
| INT    { $const = ASF::make_const_int(parse_ctx, $INT, @INT); }
| REAL   { $const = ASF::make_const_real(parse_ctx, $REAL, @REAL); }
| STRING { $const = ASF::make_const_string(parse_ctx, $STRING, @STRING); }
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
