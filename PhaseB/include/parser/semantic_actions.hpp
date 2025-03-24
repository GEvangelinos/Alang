#ifndef SEMANTIC_ACTIONS_HPP
#define SEMANTIC_ACTIONS_HPP

void stmt__break_semicolon(void);
void stmt__continue_semicolon(void);

void term__plusplus_lvalue(void *lvalue);
void term__lvalue_plusplus(void *lvalue);
void term__minusminus_lvalue(void *lvalue);
void term__lvalue_minusminus(void *lvalue);

void assignexpr__lvalue(void *lvalue);

void lvalue__id(void *lvalue, void *id);
void lvalue__local_id(void *lvalue, void *id);
void lvalue__colonblock_id(void *lvalue, void *id);
void lvalue__member(void);

void block__leftbrace(void);
void block__leftbrace_multistmt_rightbrace(void);
void block__leftbrace_rightbrace(void);

void funcdef__function_id(void *id);
void funcdef__function_id_lparen_idlist_rparen(void);
void funcdef__function_id_lparen_idlist_rparen_block(void);
void funcdef__function_lparen_idlist_rparen(void);
void funcdef__function_lparen_idlist_rparen_block(void);

void csids__id(void *id);

void whilestmt__while_lparen_expr_rparen(void);
void whilestmt__while_lparen_expr_rparen_stmt(void);

void forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen(void);
void forstmt__for_lparen_elist_semicolon_expr_semicolon_elist_rparen_stmt(void);

void returnstmt__return_semicolon(void);
void returnstmt__return_semicolon(void);
void returnstmt__return_expr_semicolon(void);

#endif /* SEMANTIC_ACTIONS_HPP */