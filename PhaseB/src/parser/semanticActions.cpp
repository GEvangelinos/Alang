#define PASS() ((void)0) // TODO: remove after completion

#include "symbolTable.hpp"
extern Alpha::SymbolTable symbolTable;

void stmt__break_semicolon(void) { PASS(); }
void stmt__continue_semicolon(void) { PASS(); }

void term__plusplus_lvalue(void *lvalue) { PASS(); }
void term__lvalue_plusplus(void *lvalue) { PASS(); }
void term__minusminus_lvalue(void *lvalue) { PASS(); }
void term__lvalue_minusminus(void *lvalue) { PASS(); }

void assignexpr__lvalue(void *lvalue) { PASS(); }

void lvalue__id(void *lvalue, void *id) { PASS(); }
void lvalue__local_id(void *lvalue, void *id) { PASS(); }
void lvalue__colonblock_id(void *lvalue, void *id) { PASS(); }
void lvalue__member(void) { PASS(); } /* TODO: What does this do? check old version for hints, or if it is even neeed. */

void block__leftbrace(void) { PASS(); }
void block__leftbrace_multistmt_rightbrace(void) { PASS(); }
void block__leftbrace_rightbrace(void) { PASS(); }

void funcdef__function_id(void *id) { PASS(); }
void funcdef__function_id_lparen_idlist_rparen(void) { PASS(); }
void funcdef__function_id_lparen_idlist_rparen_block(void) { PASS(); }
void funcdef__function_lparen_idlist_rparen(void) { PASS(); }
void funcdef__function_lparen_idlist_rparen_block(void) { PASS(); }

void csids__id(void *id) { PASS(); }

void whilestmt__while_lparen_expr_rparen(void) { PASS(); }
void whilestmt__while_lparen_expr_rparen_stmt(void) { PASS(); }

void forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen(void) { PASS(); }
void forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen_stmt(void) { PASS(); }

void returnstmt__return_semicolon(void) { PASS(); }
void returnstmt__return_expr_semicolon(void) { PASS(); }

#undef PASS