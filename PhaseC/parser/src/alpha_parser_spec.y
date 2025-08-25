/// Grammar integration
/// -------------------
/// All semantic actions delegate through the constexpr dispatcher, e.g.:
///   ss.call<"basic_builder.build_arithmetic">(Op::ADD, $lhs, $rhs, @out);
/// This keeps grammar rules declarative while preserving compile-time routing to the
/// correct subsystem method, with early-exit on error via SemanticSystem::good().

%code top
{
    // IWYU pragma: no_include <features.h>
    // IWYU pragma: no_include <stdio.h>
    // IWYU pragma: no_include <stdlib.h>
    // IWYU pragma: no_include <string.h>
    #include <parser/alpha_parser.gen.hpp>
    #include <string>                       // for basic_string, string
    #include "parser/trace_logger.hpp"      // for display_trace
    #include "parser/parser_context.hpp"    // for ParseCtx
    #include <L1_driver/semantic_system.hpp>
    using Op = alpha::ir::Opcode;
}

%code requires
{
    #define YYSTYPE ALPHA_YYSTYPE
    #define YYLTYPE ALPHA_YYLTYPE

    #include <core/source_location.hpp>
    #include <parser/internal_typedefs.hpp>

    typedef void* yyscan_t;

    namespace alpha
    {
        struct Expr;
        class FuncSymbol;
        class DiagnosticEngine;
        class DiagnosticReporter;
        class LexerCtx;
        class ParseCtx;
        class SemanticSystem;
    } // namespace alpha
}

%define api.pure full
%define api.prefix {alpha_yy}
%define parse.lac full
%define parse.error custom /* Enables custom syntax error composer (yyreport_syntax_error)*/
%define api.location.type { alpha::SourceLocation }
%code
{
   #include "core/shared_interface.hpp"
   YY_DECL;
   #include <scanner/alpha_scanner.gen.hpp>
    #include "parser_prologue_code.hpp"     // THIS MUST STAY in parser's.cpp not parser's .hpp
}
%locations

%parse-param { yyscan_t yyscanner }
%parse-param { alpha::LexerCtx &lexer_ctx }
%parse-param { alpha::LocationTracker &location_tracker }
%parse-param { alpha::DiagnosticEngine &diagnostic_engine }
%parse-param { alpha::DiagnosticReporter &dr }
%parse-param { alpha::SemanticSystem &ss }

%lex-param { yyscan_t yyscanner }
%lex-param { alpha::LexerCtx &lexer_ctx }
%lex-param { alpha::LocationTracker &location_tracker }
%lex-param { alpha::DiagnosticReporter &dr }

// Here I declare the trivial types that can be used in union.
// More complex types are stores in ParseCache of ParseCtx.
// The split is done, because we use Bison C's backend, but mine
// semantic driver is written in C++. Bison's C++ driver is more
// complex and appears to be problematic (erroneous).
%union{
    char *cstring;
    bool const_bool;
    alpha::AlphaInt const_int;
    alpha::AlphaFloat const_float;
    const alpha::FuncSymbol *const_func_symbol_ptr;
    const alpha::Expr *const_expr_ptr;
    alpha::ExprList *expr_list_ptr;
    const alpha::ExprPair *const_expr_pair_ptr;
    alpha::DictList *dict_list_ptr;

    alpha::BlockSourceLocation block_location;
}

%type  <const_expr_ptr> const
%type  <const_expr_ptr> primary
%type  <const_expr_ptr> term
%type  <const_expr_ptr> lvalue
%type  <const_expr_ptr> expr
%type  <const_expr_ptr> not_op
%type  <const_expr_ptr> and_op
%type  <const_expr_ptr> or_op
%type  <const_expr_ptr> assignExpr
%type  <const_expr_ptr> call

%type  <expr_list_ptr> expr_list
%type  <expr_list_ptr> comma_separated_exprs

%type  <const_expr_pair_ptr> dict_entry
%type  <dict_list_ptr> dict_list
%type  <const_expr_ptr> table_literal
%type  <const_expr_ptr> table_host
%type  <const_expr_ptr> table_item

%type  <const_func_symbol_ptr> func_signature
%type  <const_func_symbol_ptr> func_def
/********************************************************
%type  <expr_ptr> member
%type  <expr_ptr> call
%type  <expr_ptr> objectDef

*******************************************************/
%type  <block_location> block_loc

/**
 * By default Bison uses the bare token names (e.g. IF, METHOD_CALL)
 * in its syntax‐error messages.  If you follow a %token with
 * a quoted string, Bison will use that string instead as the
 * token’s name.  That way you get messages like:
 *
 *     syntax error, unexpected "keyword if"
 *     syntax error, unexpected "global operator ::"
 *
 * instead of
 *
 *     syntax error, unexpected IF
 *     syntax error, unexpected GLOBAL
 */

%token <cstring>        STRING "string-literal"
%token <cstring>        ID     "identifier"
%token <const_int>      INT    "integer-constant"
%token <const_float>    FLOAT  "float-constant"

/* Keyword tokens */
%token IF       "keyword if"
%token ELSE     "keyword else"
%token WHILE    "keyword while"
%token FOR      "keyword for"
%token CONTINUE "keyword continue"
%token BREAK    "keyword break"
%token FUNCTION "keyword function"
%token RETURN   "keyword return"
%token NOT      "keyword not"
%token AND      "keyword and"
%token OR       "keyword or"
%token LOCAL    "keyword local"
%token NIL      "keyword nil"
%token TRUE     "keyword true"
%token FALSE    "keyword false"

/* Operator tokens */
%token ASSIGN    "assignment operator ="
%token PLUS      "+"
%token MINUS     "-"
%token MUL       "*"
%token DIV       "/"
%token MOD       "%"
%token LT        "<"
%token GT        ">"
%token GTE       ">="
%token LTE       "<="
%token EQ        "=="
%token NEQ       "!="
%token DEC       "decrement operator --"
%token INC       "increment operator ++"

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
%token METHOD_CALL   "method-call operator .."
%token COLON         ":"
%token GLOBAL        "global operator ::"

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
/* (empty) */
| multiStmt
;

multiStmt:
  stmt
| multiStmt stmt
;

stmt:
  stmt_impl
  {
    ss.call<"reset_stmt_context">();
    ss.recover();
  }
;

stmt_impl:
  expr SEMICOLON { ss.call<"finalize_bool_expr">($expr); }
| if_stmt
| while_stmt
| for_stmt
| return_stmt SEMICOLON  { ss.recover(); }
| loop_ctrl_stmt SEMICOLON { ss.recover(); }
| block_loc
| func_def
| SEMICOLON
| error RIGHT_BRACE   { ss.recover(); yyerrok; }
;

loop_ctrl_stmt:
  BREAK    { ss.call<"control_flow_manager.manage_break">(@BREAK); }
| CONTINUE { ss.call<"control_flow_manager.manage_continue">(@CONTINUE); }
;

not_op:
  NOT expr
  {
    $expr = ss.call<"normalize_to_bool_expr">($expr);
    $not_op = ss.call<"basic_builder.build_logical_not">($expr, @not_op);
  }
;

and_op:
  expr[lhs]
  AND
  {
    $lhs = ss.call<"normalize_to_bool_expr">($lhs);
    ss.call<"basic_builder.mark_short_circuit_jump_point">();
  }
  expr[rhs]
  {
    $rhs = ss.call<"normalize_to_bool_expr">($rhs);
    $and_op = ss.call<"basic_builder.build_logical_and">($lhs, $rhs, @and_op);
  }
;

or_op:
  expr[lhs]
  OR
  {
    $lhs = ss.call<"normalize_to_bool_expr">($lhs);
    ss.call<"basic_builder.mark_short_circuit_jump_point">();
  }
  expr[rhs]
  {
    $rhs = ss.call<"normalize_to_bool_expr">($rhs);
    $or_op = ss.call<"basic_builder.build_logical_or">($lhs, $rhs, @or_op);
  }
;

expr[out]:
  assignExpr { $out = $assignExpr; }
| term       { $out = $term; }
| expr[lhs] PLUS  expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::ADD,    $lhs, $rhs, @out); }
| expr[lhs] MINUS expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::SUB,    $lhs, $rhs, @out); }
| expr[lhs] MUL   expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::MUL,    $lhs, $rhs, @out); }
| expr[lhs] DIV   expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::DIV,    $lhs, $rhs, @out); }
| expr[lhs] MOD   expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::MOD,    $lhs, $rhs, @out); }
| expr[lhs] EQ    expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_EQ,  $lhs, $rhs, @out); }
| expr[lhs] NEQ   expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_NEQ, $lhs, $rhs, @out); }
| expr[lhs] LT    expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_LT,  $lhs, $rhs, @out); }
| expr[lhs] LTE   expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_LTE, $lhs, $rhs, @out); }
| expr[lhs] GT    expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_GT,  $lhs, $rhs, @out); }
| expr[lhs] GTE   expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_GTE, $lhs, $rhs, @out); }
| and_op { $out = $and_op; }
| or_op  { $out = $or_op; }
;

/**
 * @note: finalizing at `(` expr `)`, would try to prematurely backpatch bool expr,
 * causing a shit-storm of errors. actually tried it... thank god my debug_asserts got mad af
 * leaving this docstring here so you won't try again in the future. ;p
 */
term:
  primary                     { $term = $primary; }
| LEFT_PAREN expr RIGHT_PAREN { $term = $expr; }
| not_op                      { $term = $not_op; }
| MINUS expr %prec UMINUS { $term = ss.call<"basic_builder.build_uminus">($expr, @term); }
| INC expr { $term = ss.call<"assign_builder.build_pre_inc">($expr, @term); }
| expr INC { $term = ss.call<"assign_builder.build_post_inc">($expr, @term); }
| DEC expr { $term = ss.call<"assign_builder.build_pre_dec">($expr, @term); }
| expr DEC { $term = ss.call<"assign_builder.build_post_dec">($expr, @term); }
;

assignExpr:
  expr[lhs] ASSIGN expr[rhs]
  {
    ss.call<"finalize_bool_expr">($rhs);
    $assignExpr = ss.call<"assign_builder.build_assignment">($lhs, $rhs, @assignExpr);
  }
;

primary
: const         { $primary = $const; }
| table_literal { $primary = $table_literal; }
| lvalue        { $primary = ss.call<"lvalue_resolver.resolve_lvalue_to_rvalue">($lvalue); }
| call          { $primary = ss.call<"lvalue_resolver.resolve_lvalue_to_rvalue">($call); }
| LEFT_PAREN func_def RIGHT_PAREN
  { $primary = ss.call<"function_builder.build_program_function">($func_def, @primary); }
;

lvalue:
  table_item { $lvalue = $table_item; }
| ID         { $lvalue = ss.call<"lvalue_resolver.resolve_id">($ID, @ID); }
| LOCAL ID   { $lvalue = ss.call<"lvalue_resolver.resolve_local_id">($ID, @ID); }
| GLOBAL ID  { $lvalue = ss.call<"lvalue_resolver.resolve_global_id">($ID, @ID); }
;

table_host:
  lvalue { $table_host = $lvalue; }
| call   { $table_host = $call; }

table_item:
  table_host DOT ID[member]
  { $table_item = ss.call<"table_access_builder.build_member_access">($table_host, $member, @member, @table_item); }
| table_host LEFT_BRACKET expr[index] RIGHT_BRACKET
  {
    ss.call<"finalize_bool_expr">($index);
    $table_item = ss.call<"table_access_builder.build_index_access">($table_host, $index, @table_item);
  }
;

methodCallId:
  METHOD_CALL ID { ss.call<"call_builder.update_method_call_draft">($ID, @ID);  }
;

call[invocation]:
  call[callable] LEFT_PAREN expr_list RIGHT_PAREN
  { $invocation = ss.call<"call_builder.build_call_consuming">($callable, $expr_list, @invocation); }
| lvalue LEFT_PAREN expr_list RIGHT_PAREN
  { $invocation = ss.call<"call_builder.build_call_consuming">($lvalue, $expr_list, @invocation); }
| lvalue methodCallId LEFT_PAREN expr_list RIGHT_PAREN
  { $invocation = ss.call<"call_builder.build_method_call_consuming">($lvalue, $expr_list, @invocation); }
| LEFT_PAREN func_def RIGHT_PAREN LEFT_PAREN expr_list RIGHT_PAREN
  { $invocation = ss.call<"call_builder.build_iife_call_consuming">($func_def, $expr_list, @invocation); }
;

comma_separated_exprs[out]:
  expr
  {
    ss.call<"finalize_bool_expr">($expr);
    $out = ss.call<"aggregate_builder.build_expr_list">($expr);
  }
| comma_separated_exprs[prev]  COMMA  expr
  {
    ss.call<"finalize_bool_expr">($expr);
    $out = ss.call<"aggregate_builder.extend_expr_list">($prev, $expr);
  }
;

expr_list:
/* (empty) */           { $expr_list = ss.call<"aggregate_builder.build_expr_list">(); }
| comma_separated_exprs { $expr_list = $comma_separated_exprs; }
;

/* TODO: IS this premature finalization ? like what if you do {a && b or f : x and y and z } */
/* TODO: Should we add expr finalization after color and RIGHT brace as anchor points? */
/* TODO: For some reason.. it seems more right.. But I know i have thought of that before.. and i came */
/* TODO: To the realization that this was bettter... Only that I didnt document it.. and now I still dont know for certain. */
/* FUTURE answer: it probably is correct as these exprs are the end result. expr && expr could not get matched*/
/* here for example... Anyway I am still leaving this todo here, to test it (just to be 100% certain) */
dict_entry:
  LEFT_BRACE
  expr[key]   { ss.call<"finalize_bool_expr">($key); }
  COLON
  expr[value] { ss.call<"finalize_bool_expr">($value); }
  RIGHT_BRACE { $dict_entry = ss.call<"aggregate_builder.build_expr_pair">($key, $value); }
;

dict_list[out]:
  dict_entry
  { $out = ss.call<"aggregate_builder.build_dict_list">($dict_entry); }
| dict_list[prev] COMMA dict_entry
  { $out = ss.call<"aggregate_builder.extend_dict_list">($prev, $dict_entry); }
;

table_literal:
  LEFT_BRACKET expr_list RIGHT_BRACKET
  { $table_literal = ss.call<"aggregate_builder.build_table_list_consuming">($expr_list, @table_literal); }
|
  LEFT_BRACKET dict_list RIGHT_BRACKET
  { $table_literal = ss.call<"aggregate_builder.build_table_dict_consuming">($dict_list, @table_literal); }
;

block_begin:
  LEFT_BRACE  { ss.call<"block_manager.enter_block">(); }
;

block_end:
  RIGHT_BRACE { ss.call<"block_manager.exit_block">(); }
;

block_body:
/* (empty) */
| multiStmt
;

block_loc:
  block_begin block_body block_end
  { $block_loc = ss.call<"block_manager.make_block_location">(@block_begin, @block_end); }
;

func_prefix:
  FUNCTION    { ss.call<"function_builder.update_function_draft">(@func_prefix); }
| FUNCTION ID { ss.call<"function_builder.update_function_draft">($ID, @func_prefix); }
;

funcArgs:
  ID                   { ss.call<"function_builder.collect_function_parameter">($ID, @ID); }
| funcArgs COMMA ID { ss.call<"function_builder.collect_function_parameter">($ID, @ID); }

;

funcArgList:
  LEFT_PAREN /*Empty*/ RIGHT_PAREN
| LEFT_PAREN funcArgs  RIGHT_PAREN
;

func_signature:
  func_prefix funcArgList
  { $func_signature = ss.call<"function_builder.build_program_function_entry">(@func_signature);}
;

func_def:
  func_signature block_loc
  { $func_def = ss.call<"function_builder.build_program_function_exit">($block_loc); }
;

const:
  TRUE      {  $const = ss.call<"const_builder.build_true_expr">(@TRUE); }
| FALSE     {  $const = ss.call<"const_builder.build_false_expr">(@FALSE); }
| INT       {  $const = ss.call<"const_builder.build_int_expr">($INT, @INT); }
| FLOAT     {  $const = ss.call<"const_builder.build_float_expr">($FLOAT, @FLOAT); }
| STRING    {  $const = ss.call<"const_builder.build_string_expr">($STRING, @STRING); }
| NIL       {  $const = ss.call<"const_builder.build_nil_expr">(@NIL); }
;

if_clause:
  IF LEFT_PAREN expr[condition] RIGHT_PAREN
  {
    ss.call<"finalize_bool_expr">($condition);
    ss.call<"control_flow_manager.manage_ifbranch_entry">($condition, @if_clause);
  }
;

else_clause:
  ELSE { ss.call<"control_flow_manager.manage_elsebranch_entry">(@else_clause); }
;

if_stmt:
  if_clause stmt %prec THEN       { ss.call<"control_flow_manager.manage_ifbranch_exit">(); }
| if_clause stmt else_clause stmt { ss.call<"control_flow_manager.manage_elsebranch_exit">(); }
;


while_clause:
  WHILE { ss.call<"control_flow_manager.manage_whileloop_entry">(); }
  LEFT_PAREN
  expr[condition] { ss.call<"finalize_bool_expr">($condition); }
  RIGHT_PAREN { ss.call<"control_flow_manager.manage_whileloop_condition">($condition, @while_clause); }
;

while_stmt:
  while_clause stmt { ss.call<"control_flow_manager.manage_whileloop_exit">(@while_stmt); }
;

for_clause:
  FOR
  LEFT_PAREN
  expr_list[init_list]
  SEMICOLON
  { ss.call<"control_flow_manager.mark_forloop_condition_entry">(); }
  expr[condition]
  {
    ss.call<"finalize_bool_expr">($condition);
    ss.call<"control_flow_manager.manage_forloop_condition">($condition, @condition);
  }
  SEMICOLON
  { ss.call<"control_flow_manager.mark_forloop_update_list_entry">(); }
  expr_list[update_list]
  { ss.call<"control_flow_manager.mark_forloop_update_list_exit">(@update_list); }
  RIGHT_PAREN
;

for_stmt:
  for_clause  { ss.call<"control_flow_manager.manage_forloop_entry">(); }
  stmt[body]        { ss.call<"control_flow_manager.manage_forloop_exit">(@body); }
;


return_stmt:
  RETURN
  { ss.call<"control_flow_manager.manage_return">(@RETURN); }
| RETURN  expr[retval]
  {
    ss.call<"finalize_bool_expr">($retval);
    ss.call<"control_flow_manager.manage_return">(@return_stmt, $retval);
  }
;

%%
#include "parser_epilogue_code.hpp"


