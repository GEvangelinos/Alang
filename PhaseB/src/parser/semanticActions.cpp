namespace Alpha
{
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
    void funcdef__function_id_leftparenthesis_idlist_right_parenthesis(void) { PASS(); }
    void funcdef__function_id_leftparenthesis_idlist_right_parenthesis_block(void) { PASS(); }
    void funcdef__function_leftparenthesis_idlist_rightparenthesis(void) { PASS(); }
    void funcdef__function_leftparenthesis_idlist_rightparenthesis_block(void) { PASS(); }

    void csids__id(void *id) { PASS(); }

    void whilestmt__while_leftparenthesis_expr_rightparenthesis(void) { PASS(); }
    void whilestmt__while_leftparenthesis_expr_rightparenthesis_stmt(void) { PASS(); }

    void forstmt__for_leftparenthesis_elist_semicolon_expr_semicolon_elist_rightparenthesis(void) { PASS(); }
    void forstmt__for_leftparenthesis_elist_semicolon_expr_semicolon_elist_rightparenthesis_stmt(void) { PASS(); }

    void returnstmt__return_semicolon(void) { PASS(); }
    void returnstmt__return_semicolon(void) { PASS(); }
    void returnstmt__return_expr_semicolon(void) { PASS(); }
| RETURN expr SEMI_COLON {

}