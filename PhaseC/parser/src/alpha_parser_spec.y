/// Grammar integration
/// -------------------
/// All semantic actions delegate through the constexpr dispatcher, e.g.:
///   ss.call<"basic_builder.build_arithmetic">(Op::ADD, $lhs, $rhs, @out);
/// This keeps grammar rules declarative while preserving compile-time routing to the
/// correct subsystem method, with early-exit on error via SemanticSystem::good().

%code top
{
    #include "parser_top_code.hpp"
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
        class ProgFuncSymbol;
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
    const char *cstring;
    bool const_bool;
    alpha::AlphaInt const_int;
    alpha::AlphaFloat const_float;
    const alpha::ProgFuncSymbol *const_progfunc_symbol_ptr;
    const alpha::Expr *const_expr_ptr;
    const alpha::ExprPair *const_expr_pair_ptr;

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
%type  <const_expr_ptr> assign_expr
%type  <const_expr_ptr> call
%type  <const_expr_ptr> table_literal
%type  <const_expr_ptr> table_item

%type  <const_expr_pair_ptr> dict_element

%type  <const_progfunc_symbol_ptr> func_signature
%type  <const_progfunc_symbol_ptr> func_def
%type  <block_location> block

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
/* (void) */
| multiStmt
;

multiStmt:
  stmt
| multiStmt stmt
;

/* We also recover at this point (mainly for semantic hard errors, which do not tirgger parser error). */
stmt:
  stmt_impl
  {
    ss.call<"reset_stmt_context">();
    ss.recover();
  }
;

stmt_impl:
  expr SEMICOLON { ss.call<"consume_stmt_expr">($expr); }
| if_stmt
| while_stmt
| for_stmt
| return_stmt SEMICOLON
| loop_ctrl_stmt SEMICOLON
| block
| func_def
| SEMICOLON
| error SEMICOLON   { CLEAR_ERROR_IF_NOT_IN_FORLOOP_CLAUSE(ss); }
| error RIGHT_BRACE { CLEAR_ERROR_IF_NOT_IN_TABLEDICT(ss); }
;

loop_ctrl_stmt:
  BREAK    { ss.call<"control_flow_manager.manage_break">(@BREAK); }
| CONTINUE { ss.call<"control_flow_manager.manage_continue">(@CONTINUE); }
;

not_op:
  NOT expr
  {
    $expr = ss.call<"basic_builder.prepare_logical_operand_expr">($expr);
    $not_op = ss.call<"basic_builder.build_logical_not">($expr, @not_op);
  }
;

/**
 * @note: bool exprs *must* be prepared here, before marking jumps.
 * Tried hiding it inside build_* more than once → instant chaos.
 * Don’t repeat past-me’s mistake. ;)
 */
and_op:
  expr[lhs]
  AND
  {
   $lhs = ss.call<"basic_builder.prepare_logical_operand_expr">($lhs);
   ss.call<"basic_builder.mark_short_circuit_jump_point">();
  }
  expr[rhs]
  {
    $rhs = ss.call<"basic_builder.prepare_logical_operand_expr">($rhs);
    $and_op = ss.call<"basic_builder.build_logical_and">($lhs, $rhs, @and_op);
  }
;

/**
 * @note: bool exprs *must* be prepared here, before marking jumps.
 * Tried hiding it inside build_* more than once → instant chaos.
 * Don’t repeat past-me’s mistake. ;)
 */
or_op:
  expr[lhs]
  OR
  {
    $lhs = ss.call<"basic_builder.prepare_logical_operand_expr">($lhs);
    ss.call<"basic_builder.mark_short_circuit_jump_point">();
  }
  expr[rhs]
  {
    $rhs = ss.call<"basic_builder.prepare_logical_operand_expr">($rhs);
    $or_op = ss.call<"basic_builder.build_logical_or">($lhs, $rhs, @or_op);
  }
;

expr[out]:
  assign_expr { $out = $assign_expr; }
| term        { $out = $term; }
| expr[lhs]  PLUS[op] expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::ADD,    $lhs, $rhs, @op); }
| expr[lhs] MINUS[op] expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::SUB,    $lhs, $rhs, @op); }
| expr[lhs]   MUL[op] expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::MUL,    $lhs, $rhs, @op); }
| expr[lhs]   DIV[op] expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::DIV,    $lhs, $rhs, @op); }
| expr[lhs]   MOD[op] expr[rhs] { $out = ss.call<"basic_builder.build_arithmetic">(Op::MOD,    $lhs, $rhs, @op); }
| expr[lhs]    EQ[op] expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_EQ,  $lhs, $rhs, @op); }
| expr[lhs]   NEQ[op] expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_NEQ, $lhs, $rhs, @op); }
| expr[lhs]    LT[op] expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_LT,  $lhs, $rhs, @op); }
| expr[lhs]   LTE[op] expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_LTE, $lhs, $rhs, @op); }
| expr[lhs]    GT[op] expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_GT,  $lhs, $rhs, @op); }
| expr[lhs]   GTE[op] expr[rhs] { $out = ss.call<"basic_builder.build_relational">(Op::IF_GTE, $lhs, $rhs, @op); }
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
| not_op                      { $term = $not_op; }
| LEFT_PAREN
  expr
  RIGHT_PAREN { $term = ss.call<"force_rvalue_cast">($expr, @term); }
| MINUS expr %prec UMINUS     { $term = ss.call<"basic_builder.build_uminus">($expr, @term); }
| INC expr { $term = ss.call<"assign_builder.build_pre_inc">($expr, @term); }
| expr INC { $term = ss.call<"assign_builder.build_post_inc">($expr, @term); }
| DEC expr { $term = ss.call<"assign_builder.build_pre_dec">($expr, @term); }
| expr DEC { $term = ss.call<"assign_builder.build_post_dec">($expr, @term); }
;

assign_expr:
  expr[lhs] ASSIGN expr[rhs]
  { $assign_expr = ss.call<"assign_builder.build_assignment">($lhs, $rhs, @assign_expr); }
;

primary:
  const         { $primary = $const; }
| table_literal { $primary = $table_literal; }
| lvalue        { $primary = $lvalue; }
| call          { $primary = $call; }
| LEFT_PAREN func_def RIGHT_PAREN
  { $primary = ss.call<"function_builder.forward_program_function">($func_def, @primary); }
;

lvalue:
  table_item { $lvalue = $table_item; }
| ID         { $lvalue = ss.call<"lvalue_resolver.resolve_id">($ID, @lvalue); }
| LOCAL ID   { $lvalue = ss.call<"lvalue_resolver.resolve_local_id">($ID, @lvalue); }
| GLOBAL ID  { $lvalue = ss.call<"lvalue_resolver.resolve_global_id">($ID, @lvalue); }
;

table_item:
  expr[base] DOT ID[member]
  { $table_item = ss.call<"table_access_builder.build_member_access">($base, $member, @member, @table_item); }
| expr[base] LEFT_BRACKET expr[subscript] RIGHT_BRACKET
  { $table_item = ss.call<"table_access_builder.build_subscript_access">($base, $subscript, @table_item); }
;

method_call_id:
  METHOD_CALL ID { ss.call<"call_builder.update_method_call_draft">($ID, @ID); }
;

begin_arg_list:
  LEFT_PAREN  { ss.call<"call_builder.init_call">(); }
;

end_arg_list:
  RIGHT_PAREN
;

arg_list:
  begin_arg_list expr_list end_arg_list
;

call[invocation]:
  call[callable] arg_list
  { $invocation = ss.call<"call_builder.build_call_consuming">($callable, @invocation); }
| lvalue arg_list
  { $invocation = ss.call<"call_builder.build_call_consuming">($lvalue, @invocation); }
| lvalue[method_host] method_call_id arg_list
  { $invocation = ss.call<"call_builder.build_method_call_consuming">($method_host, @invocation); }
| LEFT_PAREN func_def RIGHT_PAREN arg_list
  { $invocation = ss.call<"call_builder.build_iife_call_consuming">($func_def, @arg_list); }
;

cs_exprs:
  expr  { ss.call<"commit_expr_of_elist">($expr); }
| cs_exprs
  COMMA
  expr  { ss.call<"commit_expr_of_elist">($expr); }
;

expr_list:
/* (void) */
| cs_exprs
;

dict_element:
  LEFT_BRACE
  { ss.call<"aggregate_builder.begin_dict_entry">(); }
  expr[key]
  COLON
  expr[value]
  RIGHT_BRACE
  {
    ss.call<"aggregate_builder.end_dict_entry">();
    ss.call<"aggregate_builder.commit_dict_element">($key, $value, @dict_element);
  }
;

/* Mirroring expr_list colletion */
cs_dict_elements:
  dict_element
| dict_list COMMA dict_element

dict_list:
  cs_dict_elements
;

table_literal_begin:
  LEFT_BRACKET
  { ss.call<"aggregate_builder.init_table_literal">(); }
;

table_literal_end:
  RIGHT_BRACKET
;

table_literal:
  table_literal_begin expr_list table_literal_end
  { $table_literal = ss.call<"aggregate_builder.finalize_table_literal">(@table_literal); }
|
  table_literal_begin dict_list table_literal_end
  { $table_literal = ss.call<"aggregate_builder.finalize_table_literal">(@table_literal); }
;

block_begin:
  LEFT_BRACE { ss.call<"block_manager.enter_block">(); }
;

block_end:
  RIGHT_BRACE { ss.call<"block_manager.exit_block">(); }
;

block_body:
/* (void) */
| multiStmt
;

block:
  block_begin block_body block_end
  { $block = ss.call<"block_manager.make_block_location">(@block_begin, @block_end); }
;

func_prefix:
  FUNCTION    { ss.call<"function_builder.update_function_draft">(); }
| FUNCTION ID { ss.call<"function_builder.update_function_draft">($ID); }
;

func_params:
  ID                   { ss.call<"function_builder.collect_function_parameter">($ID, @ID); }
| func_params COMMA ID { ss.call<"function_builder.collect_function_parameter">($ID, @ID); }

;

funcArgList:
  LEFT_PAREN /* (void) */ RIGHT_PAREN
| LEFT_PAREN func_params  RIGHT_PAREN
| LEFT_PAREN    error     RIGHT_PAREN { CLEAR_ERROR_IF_IN_FUNC_PARAM_LIST(ss); }
;

func_signature:
  func_prefix funcArgList
  { $func_signature = ss.call<"function_builder.build_program_function_entry">(@func_signature); }
;

func_def:
  func_signature block
  { $func_def = ss.call<"function_builder.build_program_function_exit">($block); }
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
  { ss.call<"control_flow_manager.manage_ifbranch_entry">($condition, @if_clause); }
;

else_clause:
  ELSE { ss.call<"control_flow_manager.manage_elsebranch_entry">(@else_clause); }
;

if_stmt:
  if_clause stmt %prec THEN       { ss.call<"control_flow_manager.manage_ifbranch_exit">(); }
| if_clause stmt else_clause stmt { ss.call<"control_flow_manager.manage_elsebranch_exit">(); }
;


while_clause:
  WHILE
  { ss.call<"control_flow_manager.manage_whileloop_entry">(); }
  LEFT_PAREN
  expr[condition]
  RIGHT_PAREN
  { ss.call<"control_flow_manager.manage_whileloop_condition">($condition, @while_clause); }
;

while_stmt:
  while_clause stmt { ss.call<"control_flow_manager.manage_whileloop_exit">(@while_stmt); }
;

for_clause:
  FOR
  { ss.call<"control_flow_manager.enter_forloop_clause">(); }
  LEFT_PAREN
  expr_list[init_list]
  SEMICOLON
  { ss.call<"control_flow_manager.mark_forloop_condition_entry">(); }
  expr[condition]
  { ss.call<"control_flow_manager.manage_forloop_condition">($condition, @condition); }
  SEMICOLON
  { ss.call<"control_flow_manager.mark_forloop_update_list_entry">(); }
  expr_list[update_list]
  {
    ss.call<"control_flow_manager.mark_forloop_update_list_exit">(@update_list);
  }
  RIGHT_PAREN
  { ss.call<"control_flow_manager.exit_forloop_clause">(); }
| FOR error RIGHT_PAREN
  {
    CLEAR_ERROR(ss);
    ss.call<"control_flow_manager.mark_bad_forloop_clause">();
  }
;

for_stmt:
  for_clause { ss.call<"control_flow_manager.manage_forloop_entry">(); }
  stmt[body] { ss.call<"control_flow_manager.manage_forloop_exit">(@body); }
;


return_stmt:
  RETURN
  { ss.call<"control_flow_manager.manage_return">(@RETURN); }
| RETURN  expr[retval]
  { ss.call<"control_flow_manager.manage_return">(@return_stmt, $retval); }
;

%%
#include "parser_epilogue_code.hpp"


