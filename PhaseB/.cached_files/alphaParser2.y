%{
#define INSIDE_BISON_FILE
#include <string>
#include <list>
#include <iostream>
#include <stdexcept>
#include "alphaScanner.hpp"
#include "alphaDefs.hpp"
#include "errorTracker.hpp"
    bool isFunctionBlock = false;
    bool lvalueIsMember = false;
    int functionDepthCounter = 0;
    %}

%code requires
{
#include "symbolTable.hpp"
    extern Alpha::SymbolTable symbolTable;
}

%define api.prefix {alpha_yy}
%define parse.error detailed    /* Enable detailed error messages */


%start program

%union{
    char *unionStringLiteral;
    char *unionId;
    long unionIntConst;
    double unionRealConst;
    Alpha::SymbolTableEntry *unionLvalue;
 }

%token <unionStringLiteral> STRING_LITERAL
%token <unionId>ID
%token <unionIntConst> INT_CONST
%token <unionRealConst> REAL_CONST
%type <unionLvalue> lvalue

 /* Keyword tokens */
%token IF 
%token ELSE
%token WHILE
%token FOR
%token FUNCTION
%token RETURN
%token BREAK
%token CONTINUE
%token AND
%token NOT
%token OR
%token LOCAL
%token TRUE
%token FALSE
%token NIL

 /* Operator tokens */
%token ASSIGN
%token PLUS
%token MINUS
%token MUL
%token DIV
%token MOD
%token EQUAL
%token NOT_EQUAL
%token INC
%token DEC 
%token GREATER_THAN
%token LESS_THAN
%token GREATER_THAN_OR_EQUAL
%token LESS_THAN_OR_EQUAL

 /* Punctuation tokens */
%token LEFT_BRACE 
%token RIGHT_BRACE
%token LEFT_BRACKET 
%token RIGHT_BRACKET
%token LEFT_PARENTHESIS
%token RIGHT_PARENTHESIS
%token SEMI_COLON
%token COMMA
%token COLON
%token GLOBAL
%token DOT
%token METHOD_CALL

 /* Priorities */
%right ASSIGN
%left OR
%left AND
%nonassoc EQUAL NOT_EQUAL
%nonassoc GREATER_THAN GREATER_THAN_OR_EQUAL LESS_THAN LESS_THAN_OR_EQUAL
%left PLUS MINUS
%left MUL DIV MOD
%right NOT INC DEC
%left DOT METHOD_CALL
%left LEFT_BRACKET RIGHT_BRACKET
%left LEFT_PARENTHESIS RIGHT_PARENTHESIS

%precedence THEN
%precedence ELSE

 /* Grammar rules: */
%%
program         : /* Void */                                    {displayLog("program", "");}
|  multi_stmt                                   {displayLog("program", "multi_stmt");}
;

stmt            : expr SEMI_COLON                               {displayLog("stmt","expr SEMI_COLON");}
| ifstmt                                        {displayLog("stmt","ifstmt");}
| whilestmt                                     {displayLog("stmt","whilestmt");}
| forstmt                                       {displayLog("stmt","forstmt");}
| returnstmt                                    {displayLog("stmt","returnstmt");}
| BREAK SEMI_COLON {
    displayLog("stmt","BREAK SEMI_COLON");
    if (symbolTable.getCurrentLoopDepthCounter() == 0)
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, "Keyword break was used outside a loop block."));
 }
| CONTINUE SEMI_COLON {
    displayLog("stmt","CONTINUE SEMI_COLON");
    if (symbolTable.getCurrentLoopDepthCounter() == 0)
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, "Keyword continue was used outside a loop block."));
 }
| block                                         {displayLog("stmt","block");}
| funcdef                                       {displayLog("stmt","funcdef");}
| SEMI_COLON                                    {displayLog("stmt","SEMI_COLON");}
;

multi_stmt      : stmt                                          {displayLog("multi_stmt", "stmt");}
| stmt multi_stmt                               {displayLog("multi_stmt", "stmt multi_stmt");}
;

expr            : assignexpr                                    {displayLog("expr", "assignexpr");}
| expr PLUS expr                                {displayLog("expr", "expr PLUS expr");}
| expr MINUS expr                               {displayLog("expr", "expr MINUS expr");}
| expr MUL expr                                 {displayLog("expr", "expr MUL expr");}
| expr DIV expr                                 {displayLog("expr", "expr DIV expr");}
| expr MOD expr                                 {displayLog("expr", "expr MOD expr");}
| expr GREATER_THAN expr                        {displayLog("expr", "expr GREATER_THAN expr");}
| expr GREATER_THAN_OR_EQUAL expr               {displayLog("expr", "expr GREATER_THAN_OR_EQUAL expr");}
| expr LESS_THAN expr                           {displayLog("expr", "expr LESS_THAN expr");}
| expr LESS_THAN_OR_EQUAL expr                  {displayLog("expr", "expr LESS_THAN_OR_EQUAL expr");}
| expr EQUAL expr                               {displayLog("expr", "expr EQUAL expr ");}
| expr NOT_EQUAL expr                           {displayLog("expr", "expr NOT_EQUAL expr");}
| expr AND expr                                 {displayLog("expr", "expr AND expr");}
| expr OR expr                                  {displayLog("expr", "expr OR expr");}
| term                                          {displayLog("expr", "term");}
;

term            : LEFT_PARENTHESIS expr RIGHT_PARENTHESIS       {displayLog("term", "LEFT_PARENTHESIS expr RIGHT_PARENTHESIS");}
| MINUS expr                                    {displayLog("term", "MINUS expr");}
| NOT expr                                      {displayLog("term", "NOT expr");}
| INC lvalue {
    displayLog("term", "INC lvalue");
    if ($2 != nullptr && (($2->getType() == Alpha::SymbolType::LIBFUNC || $2->getType() == Alpha::SymbolType::USERFUNC)))
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, std::string("Operator ++  can not be used on function ") + $2->getName()));
 }
| lvalue  INC {
    displayLog("term", "lvalue INC");
    if ($1 != nullptr && (($1->getType() == Alpha::SymbolType::LIBFUNC || $1->getType() == Alpha::SymbolType::USERFUNC)))
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, std::string("Operator ++  can not be used on function ") + $1->getName()));
 }
| DEC lvalue {
    displayLog("term", "DEC lvalue");
    if ($2 != nullptr && (($2->getType() == Alpha::SymbolType::LIBFUNC || $2->getType() == Alpha::SymbolType::USERFUNC)))
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, std::string("Operator --  can not be used on function ") + $2->getName()));
 }
| lvalue DEC {
    displayLog("term", "lvalue DEC");
    if ($1 != nullptr && (($1->getType() == Alpha::SymbolType::LIBFUNC || $1->getType() == Alpha::SymbolType::USERFUNC)))
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, std::string("Operator --  can not be used on function ") + $1->getName()));
 }
| primary                                       {displayLog("term", "primary");}
;
//displaySyntaxrError("Symbol is already declared, but is not a variable.", alpha_yylineno);

assignexpr      : lvalue
{
    if (!lvalueIsMember && $1 != nullptr && ($1->getType() == Alpha::SymbolType::LIBFUNC || $1->getType() == Alpha::SymbolType::USERFUNC)) 
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, std::string($1->getName() + " is a function, can not assign to it.")));
    lvalueIsMember = false;
}
ASSIGN expr                            {displayLog("assignexpr", "lvalue ASSIGN expr");}
;

primary         : lvalue                                        {displayLog("primary", "lvalue");}
| call                                          {displayLog("primary", "call");}
| objectdef                                     {displayLog("primary", "objectdef");}
| LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS    {displayLog("primary", "LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS");}
| const                                         {displayLog("primary", "const");}
;

lvalue          : ID {
    displayLog("lvalue", "ID");
    auto resultPair = symbolTable.lookUpSymbol($1);
    if (resultPair.first == Alpha::OperationResult::SymbolNotFound)
	{
	    symbolTable.insertVariable($1, alpha_yylineno);
	    resultPair = symbolTable.lookUpVariable($1);
	    if (resultPair.first != Alpha::OperationResult::Success)
		throw std::runtime_error("Insertion of a variable failed after duplicate check.");
	}
    else if (resultPair.first == Alpha::OperationResult::SymbolOutsideFunction)
	{
	    symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										     alpha_yylineno, 0, std::string("Variable ") + $1 + " is declared outside of current function scope."));
	}
    $$ = resultPair.second;
}
| LOCAL ID {
    displayLog("lvalue","LOCAL ID");
    Alpha::SymbolTableEntry *entry = symbolTable.lookUpCurrentScope($2);
    if (symbolTable.isLibraryFunction($2))
	{
	    symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										     alpha_yylineno, 0, std::string($2) + " shadows library function"));
	    entry = nullptr;
	}
    else if (entry == nullptr) // if control reached here, it is not a LIBFUNC.
	{
	    symbolTable.insertVariable($2, alpha_yylineno);
	    entry = symbolTable.lookUpVariable($2).second;
	    if (entry == nullptr)
		throw std::runtime_error("Insertion of a variable failed after duplicate check");
	}
    $$ = entry;
}
| GLOBAL ID {
    displayLog("lvalue","GLOBAL ID");
    $$ = symbolTable.lookUpGlobalScope($2);
    if ($$ == nullptr)
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, std::string("::") + std::string($2) + " not found in global scope"));
}
| member{
    displayLog("lvalue","member");
    lvalueIsMember = true;
}
;

member          : lvalue DOT ID                                 {displayLog("member","lvalue DOT ID");}
| lvalue LEFT_BRACKET expr RIGHT_BRACKET        {displayLog("member","lvalue LEFT_BRACKET expr RIGHT_BRACKET");}
| call DOT ID                                   {displayLog("member","call DOT ID");}
| call LEFT_BRACKET expr RIGHT_BRACKET          {displayLog("member","CALL LEFT_BRACKET expr RIGHT_BRACKET");}
;

call            : call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {displayLog("call","call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
| lvalue callsuffix                             {displayLog("call","lvalue callsuffix");}
| LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS 
LEFT_PARENTHESIS elist RIGHT_PARENTHESIS      {displayLog("call","LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
;

callsuffix      : normcall                                      {displayLog("callsuffix","normcall");}
| methodcall                                    {displayLog("callsuffix","methodcall");}
;

normcall        : LEFT_PARENTHESIS elist RIGHT_PARENTHESIS      {displayLog("normcall","LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
;

methodcall      : METHOD_CALL ID LEFT_PARENTHESIS elist 
RIGHT_PARENTHESIS                             {displayLog("methodcall","METHOD_CALL ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
;

expr_list       : expr                                          {displayLog("expr_list", "expr");}
| expr COMMA expr_list                          {displayLog("expr_list", "expr COMMA expr_list");}
;

elist           : expr_list                                     {displayLog("elist", "expr_list");}
| /* Void */                                    {displayLog("elist", "");}
;

objectdef       : LEFT_BRACKET elist RIGHT_BRACKET              {displayLog("objectdef", "LEFT_BRACKET elist RIGHT_BRACKET");}
| LEFT_BRACKET indexed RIGHT_BRACKET            {displayLog("objectdef", "LEFT_BRACKET indexed RIGHT_BRACKET");}
;

indexed         : indexedelem_list                              {displayLog("indexed", "indexedelem_list");}     
; /* | EMPTY (Removed to avoid conflict.. else I could use %expect 2. */

indexedelem     : LEFT_BRACE expr COLON expr RIGHT_BRACE        {displayLog("indexedelem", "LEFT_BRACE expr COLON expr RIGHT_BRACE");}       
;

block           : LEFT_BRACE
{
    symbolTable.incrementScope(isFunctionBlock); // isFunctionBlock is a bool: true, or false.
    isFunctionBlock = false; // Reset flag.
}
multi_stmt
RIGHT_BRACE
{
    symbolTable.decrementScope();
    displayLog("block", "LEFT_BRACE multi_stmt RIGHT_BRACE");
}
| LEFT_BRACE
{
    // There might be FORMAL arguments to this function.
    // And because member function incrementScope() declares them, we call it.
    symbolTable.incrementScope(isFunctionBlock); // isFunctionBlock is a bool: true, or false.
    isFunctionBlock = false; // Reset flag.
}
RIGHT_BRACE
{
    symbolTable.decrementScope();
    displayLog("block", "LEFT_BRACE RIGHT_BRACE");
}
;

funcdef         : FUNCTION
ID
{Alpha::Function::nameOfLastFunction = $2;}
LEFT_PARENTHESIS
idlist
RIGHT_PARENTHESIS 
{
    Alpha::SymbolTableEntry *currentScopeEntry = symbolTable.lookUpCurrentScope(Alpha::Function::nameOfLastFunction);
    if (symbolTable.isLibraryFunction(Alpha::Function::nameOfLastFunction))
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, std::string("Redefinition of library function ") + Alpha::Function::nameOfLastFunction + " is prohibited"));
    else if (currentScopeEntry && currentScopeEntry->getType() == Alpha::SymbolType::USERFUNC)
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0 ,std::string("Function ") + Alpha::Function::nameOfLastFunction + " is already defined in this scope. Can not redifine."));
    else if (currentScopeEntry) // We found a symbol, and it was a LIBFUNC nor a USERFUNC, thus it is a variable
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, std::string(Alpha::Function::nameOfLastFunction) + " is already defined as a variable."));
    else
	isFunctionBlock = (symbolTable.insertFunction(Alpha::Function::nameOfLastFunction, alpha_yylineno, Alpha::SymbolType::USERFUNC, Alpha::Function::idList) == Alpha::OperationResult::Success);
    functionDepthCounter++;
}
block
{
    displayLog("funcdef", "FUNCTION ID LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block");
    functionDepthCounter--;
}
| FUNCTION
LEFT_PARENTHESIS
idlist
RIGHT_PARENTHESIS
{
    symbolTable.insertNamelessFunction(alpha_yylineno, Alpha::SymbolType::USERFUNC, Alpha::Function::idList);
    isFunctionBlock = true;
    functionDepthCounter++;
}
block
{
    displayLog("funcdef", "FUNCTION LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block");
    functionDepthCounter--;
}
;

const           : INT_CONST                                     {displayLog("const", "INT_CONST");}
| REAL_CONST                                    {displayLog("const", "REAL_CONST");}
| STRING_LITERAL                                {displayLog("const", "STRING_LITERAL");}
| NIL                                           {displayLog("const", "NIL");}
| TRUE                                          {displayLog("const", "TRUE");}
| FALSE                                         {displayLog("const", "FALSE");}
;

cs_ids          : ID {
    Alpha::Function::idList.push_back(std::string($1));
    displayLog("cs_ids", "ID");
}
| ID {
    Alpha::Function::idList.push_back(std::string($1));
} COMMA cs_ids                                  {displayLog("cs_ids", "ID COMMA cs_ids");}
;

idlist          : cs_ids                                        {displayLog("idlist", "cs_ids");}
|                                               {displayLog("idlist", "");}
;

ifstmt          : IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
stmt %prec THEN                               {displayLog("ifstmt", "IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt");}
| IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS 
stmt ELSE  stmt                               {displayLog("ifstmt", "IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt");}
;

whilestmt       : WHILE LEFT_PARENTHESIS expr 
RIGHT_PARENTHESIS
{
    symbolTable.incrementCurrentLoopDepthCounter();
}
stmt
{
    displayLog("whilestmt", "WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt"); 
    symbolTable.decrementCurrentLoopDepthCounter();
}
;

forstmt         : FOR LEFT_PARENTHESIS elist SEMI_COLON expr 
SEMI_COLON elist RIGHT_PARENTHESIS 
{
    symbolTable.incrementCurrentLoopDepthCounter();
}
stmt
{
    displayLog("forstmt", "FOR LEFT_PARENTHESIS elist SEMI_CLON expr SEMI_CLON elist RIGHT_PARENTHESIS stmt");
    symbolTable.decrementCurrentLoopDepthCounter();
}
;

returnstmt      : RETURN SEMI_COLON {
    displayLog("returnstmt", "RETURN SEMI_COLON");
    //Check if in function block.
    if(functionDepthCounter == 0)
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, "Keyword return was used outside a function block."));
}
| RETURN expr SEMI_COLON {
    displayLog("returnstmt", "RETURN expr SEMI_COLON");
    //Check if in function block.
    if(functionDepthCounter == 0)
	symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(
										 alpha_yylineno, 0, "Keyword return was used outside a function block."));
}
;

indexedelem_list        : indexedelem                           {displayLog("indexedelem_list", "indexedelem");}
| indexedelem COMMA indexedelem_list    {displayLog("indexedelem_list", "indexedelem COMMA indexedelem_list");}
;
