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
  { sm.multiStmt__stmt(); }
| stmt
  { sm.multiStmt__stmt(); }
 multiStmt
;

stmt
: expr SEMICOLON {  sm.backpatch_bool_expr($expr, @expr); }
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
:  BREAK    { sm.loopCtrlStmt__break(@BREAK); }
| CONTINUE { sm.loopCtrlStmt__continue(@CONTINUE); }
;

expr[result]
: assignExpr
| expr[left] PLUS  expr[right] { $result = sb.make_arithmetic(AOP::ADD, $left, $right, @result, @left, @right); }
| expr[left] MINUS expr[right] { $result = sb.make_arithmetic(AOP::SUB, $left, $right, @result, @left, @right); }
| expr[left] MUL   expr[right] { $result = sb.make_arithmetic(AOP::MUL, $left, $right, @result, @left, @right); }
| expr[left] DIV   expr[right] { $result = sb.make_arithmetic(AOP::DIV, $left, $right, @result, @left, @right); }
| expr[left] MOD   expr[right] { $result = sb.make_arithmetic(AOP::MOD, $left, $right, @result, @left, @right); }
| expr[left] GT    expr[right] { $result = sb.make_relational(AOP::IF_GREATER, $left, $right,@result, @left, @right); }
| expr[left] GTE   expr[right] { $result = sb.make_relational(AOP::IF_GREATEREQ, $left, $right,@result, @left, @right); }
| expr[left] LT    expr[right] { $result = sb.make_relational(AOP::IF_LESS, $left, $right,@result, @left, @right); }
| expr[left] LTE   expr[right] { $result = sb.make_relational(AOP::IF_LESSEQ, $left, $right,@result, @left, @right); }
| expr[left] EQ    expr[right] { $result = sb.make_relational(AOP::IF_EQ, $left, $right,@result, @left, @right); }
| expr[left] NEQ   expr[right] { $result = sb.make_relational(AOP::IF_NOTEQ, $left, $right,@result, @left, @right); }
| expr[left] AND 
  {
     $left = sb.convert_to_boolean($left, @left); 
  }
  saveNextQuadHook expr[right] 
  { 
    $right = sb.convert_to_boolean($right, @right);   
    $result = sb.make_logical_and($left, $right, @result, @left, @right);
  }
| expr[left] OR
  {
    $left = sb.convert_to_boolean($left, @left); 
  } 
  saveNextQuadHook expr[right]
  {
    $right = sb.convert_to_boolean($right, @right);   
    $result = sb.make_logical_or($left, $right, @result, @left, @right); 
  }
| term { $result = $term; }
;

saveNextQuadHook
: { sm.saveNextQuadHook(); }  
;

term:
  LEFT_PAREN expr RIGHT_PAREN { $term = $expr; }
| MINUS expr %prec UMINUS     { $term = sb.make_uminus($expr, @term, @expr); }
| NOT expr                    { $term = sb.make_logical_not($expr, @term); }
| INC lvalue { sm.term__inc_lvalue($term, $lvalue, @term); }
| lvalue INC { sm.term__lvalue_inc($term, $lvalue, @term); }
| DEC lvalue { sm.term__dec_lvalue($term, $lvalue, @term); }
| lvalue DEC { sm.term__lvalue_dec($term, $lvalue, @term); }
| primary { $term = $primary; }
;

assignExpr:
  lvalue ASSIGN expr  
  { 
    sm.backpatch_bool_expr($expr, @expr);
    $assignExpr = sb.resolve_assign_expr($lvalue,$expr, @ASSIGN); 
  }
;

primary:
  lvalue { $primary = sb.resolve_lvalue_to_primary($lvalue); }
| call // TODO do I need to forward here?
| objectDef // TODO do you need to forward. // TODO2: Find ALL places you need to forward
| LEFT_PAREN funcDef RIGHT_PAREN
  { $primary = sb.make_program_function($funcDef); }
| const  { $primary = $const; }
;


lvalue:
  ID        { sm.lvalue__id($lvalue, $ID, @ID);  std::cout << "4.PARSER: ID ==  " << $ID << std::endl; }
| LOCAL ID  { sm.lvalue__local_id($lvalue, $ID, @ID); std::cout << "5.PARSER: ID ==  " << $ID << std::endl;} 
| GLOBAL ID { sm.lvalue__global_id($lvalue, $ID, @ID); std::cout << "6.PARSER: ID ==  " << $ID << std::endl;}
| member { $lvalue = $member; }
;

tableItem:
  lvalue DOT ID
  { $tableItem = sb.make_table_item($lvalue, $ID, @tableItem ,@ID); } 
| lvalue LEFT_BRACKET expr RIGHT_BRACKET 
  {
    sm.backpatch_bool_expr($expr, @expr);
    /*TODO  extract sub rule to bracketed expression. an backpatch expr there. TODO2:
    Try to centralize expr backpatching... So far everything is scatter in different rules.
    Maybe you can make a rule with all places expressions are needed. and backpatching is also needed. */
    $tableItem = sb.make_table_item($lvalue, $expr, @tableItem);
  }
| call DOT ID { $tableItem = sb.make_table_item($call, $ID, @call, @ID); }
| call LEFT_BRACKET expr RIGHT_BRACKET 
  {
    sm.backpatch_bool_expr($expr, @expr);
    $tableItem = sb.make_table_item($call, $expr, @tableItem);
  }
;

member:
  tableItem { $member = $tableItem; /* USELESS INDIRECTION? */  /*TODO originally there where tableitem here*/}
;

methodCallId:
  METHOD_CALL ID 
  { sm.methodCallId__methodcall_id($ID, @ID, @methodCallId);}
;

//*TODO: ADD normal_call and Method_call and pass needed variable trhoguh a Call struct!! */
call[invocation]:
  call[callable] LEFT_PAREN elist RIGHT_PAREN // <------------------------------------ CHAIN_CALL
  { $invocation = sb.make_call($callable, $elist, @invocation); }
| lvalue LEFT_PAREN elist RIGHT_PAREN // <-------------------------------------------- NORMAL_CALL
  { $invocation = sb.make_normal_call($lvalue, $elist, @invocation); }
| lvalue methodCallId LEFT_PAREN elist RIGHT_PAREN // <------------------------------- METHOD_CALL
  { $invocation = sb.make_method_call($lvalue, $elist, @invocation); }
| LEFT_PAREN funcDef RIGHT_PAREN LEFT_PAREN elist RIGHT_PAREN // <---------------------IIFE_CALL
  { $invocation = sb.make_iife_call($funcDef, $elist, @invocation); }
;

exprList[head]:
  expr 
  { 
    sm.backpatch_bool_expr($expr, @expr);
    $head = sb.make_expr_list_with($expr, @expr); 
  }
| expr COMMA exprList[tail] 
  {
    sm.backpatch_bool_expr($expr, @expr);
    $head = sb.extend_expr_list_with($tail, $expr, @expr);
  }
;

elist:
  /* (empty) */ { $elist = sb.make_empty_expr_list(); }
| exprList      { $elist = $exprList; }
;

tableList:
  LEFT_BRACKET elist RIGHT_BRACKET
  { $tableList = sb.make_table_list($elist, @tableList); }
;

tableDict:
  LEFT_BRACKET indexed RIGHT_BRACKET
  { $tableDict = sb.make_table_dict($indexed, @tableDict); }
;

objectDef:
  tableList { $objectDef = $tableList; }
| tableDict { $objectDef = $tableDict; }
;

indexed:
  indexedElemList { $indexed = $indexedElemList; }
;

indexedElemList[head]:
  indexedElem
  { $head = sb.make_dict_list_with($indexedElem); }
| indexedElem COMMA indexedElemList[tail]
  { $head = sb.extend_dict_list_with($tail, $indexedElem); }
;

indexedElem:
  LEFT_BRACE expr[key] COLON
  { sm.backpatch_bool_expr($key, @key); }
  expr[value] 
  RIGHT_BRACE
  {
    sm.backpatch_bool_expr($value, @value); 
    $indexedElem = sb.make_expr_pair($key, $value); 
  }
;

blockBegin:
  LEFT_BRACE  
  { 
    sm.blockBegin__lbrace();
    $blockBegin = @LEFT_BRACE;
  }
;

blockEnd:
  RIGHT_BRACE 
  { 
    sm.blockEnd__rbrace();
    $blockEnd = @RIGHT_BRACE;
  }
;

block:
  blockBegin multiStmt  blockEnd   
  { $block = sb.make_block_location($blockBegin, $blockEnd); }
| blockBegin blockEnd
  { $block = sb.make_block_location($blockBegin, $blockEnd); }
;


funcPrefix:
  FUNCTION    { sm.funcPrefix__function(@FUNCTION); std::cout << "I PASSED FROM HERE 1"<< std::endl;}
| FUNCTION ID { sm.funcPrefix__function_id($ID, @ID); std::cout << "I PASSED FROM HERE 2"<< std::endl;}
;

funcArgs:
  ID { sm.funcArgs__id($ID, @ID); std::cout << "I PASSED FROM HERE 3"<< std::endl;}
| ID { sm.funcArgs__id($ID, @ID); std::cout << "I PASSED FROM HERE 4"<< std::endl;} COMMA funcArgs
;

funcArgList:
  LEFT_PAREN /*Void*/ RIGHT_PAREN {std::cout << "I PASSED FROM HERE 5"<< std::endl;}
| LEFT_PAREN funcArgs  RIGHT_PAREN {std::cout << "I PASSED FROM HERE 6"<< std::endl;}
;

funcSignature:
  funcPrefix{  std::cout << "I PASSED FROM HERE 9"<< std::endl;} funcArgList // need funcPREFIX ID here
  {
    std::cout << "I PASSED FROM HERE 7"<< std::endl;
    sm.funcSignature__funcPrefix_funcArgList($funcSignature); }
;

funcDef:
  funcSignature block 
  { 
    std::cout << "I PASSED FROM HERE 8"<< std::endl;
    // TODO to much inderection remove funcSignature rule and MERGE... 
    sm.funcDef__funcSignature_block($block); 
    $funcDef = $funcSignature;
  }
;

const:
  NIL    { $const = sb.make_const_nil(@NIL); }
| TRUE   { $const = sb.make_const_true(@TRUE); }
| FALSE  { $const = sb.make_const_false(@FALSE); }
| INT    { $const = sb.make_const_int($INT, @INT); }
| REAL   { $const = sb.make_const_real($REAL, @REAL); }
| STRING {

//TODO. STOP LEXER FROM COMPYING THE STRING.. IT USESLESS and MAKES US NEED EXTRA Deaclocation bookeeping 

   $const = sb.make_const_string($STRING, @STRING); delete[] $STRING; $STRING = nullptr; }
;

ifPrefix
: IF LEFT_PAREN expr RIGHT_PAREN 
  { 
    sm.backpatch_bool_expr($expr, @expr);
    sm.ifPrefix__if_lparen_expr_rparen($expr, @expr);
  }
;
elsePrefix
: ELSE { sm.elsePrefix__else(@ELSE); }
;

ifStmt
: ifPrefix stmt %prec THEN { sm.ifStmt__ifPrefix_stmt_then(); }
| ifPrefix stmt elsePrefix stmt  { sm.ifStmt__ifPrefix_stmt_elsePrefix_stmt(); }
;

whileStart:
WHILE { sm.whileStart__while(); }
;

whileCondition:
  LEFT_PAREN expr
  { sm.backpatch_bool_expr($expr, @expr); }
  RIGHT_PAREN
  { sm.whileCondition__lparen_expr_rparen($expr, @expr, @whileCondition); }
;

whileHeader:
  whileStart
  whileCondition
;

whileStmt:
  whileHeader
  { sm.whileStmt__whileHeader(); }
  stmt
  { sm.whileStmt__whileHeader_stmt(@whileStmt); }
;

N1: { sm.N(@N1,1); };
N2: { sm.N(@N2,2); };
N3: { sm.N(@N3,3); };

M: { sm.M(); };

forHeader:
  FOR
  LEFT_PAREN
  elist
  SEMICOLON
  M
  expr
  { sm.backpatch_bool_expr($expr, @expr); }
  SEMICOLON
  {
    sm.forHeader__for_lparen_elist_semicolon_m_expr_semicolon($expr, @expr);
  }
  N1
  elist
  RIGHT_PAREN
;

forStmt:
  forHeader
  N2
  { sm.forStmt__forHeader(); } 
  stmt
  N3
  { 
    sm.forStmt__forHeader_stmt();
  }
;

funcCtrlStmt: //OK
  RETURN { sm.funcCtrlStmt__return(@RETURN); }
;

returnStmt: //OK
  funcCtrlStmt
| funcCtrlStmt expr  { sm.backpatch_bool_expr($expr, @expr); }
;


%%
