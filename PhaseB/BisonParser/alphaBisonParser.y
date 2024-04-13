%{
        #define INSIDE_BISON_FILE
        #include <string>
        #include <list>
        #include <iostream>
        #include <stdexcept>
        #include "../GeneratedFiles/alphaFlexScanner.hpp"
        #include "../alphaDefs.hpp"
        bool isFunctionBlock = false;
%}

%code requires{
        #include "../symbolTable.hpp"
        extern Alpha::SymbolTable symbolTable;
}

%output "alphaBisonParser.cpp"
%define api.prefix {alpha_yy}
  /* %parse-param {std::list<Alpha::SymbolTableEntry} */
%defines
%expect 1


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
%token PLUS_PLUS
%token MINUS_MINUS
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
%token COLON_BLOCK
%token DOT
%token DDOT

/* Priorities */
%right ASSIGN
%left OR
%left AND
%nonassoc EQUAL NOT_EQUAL
%nonassoc GREATER_THAN GREATER_THAN_OR_EQUAL LESS_THAN LESS_THAN_OR_EQUAL
%left PLUS MINUS
%left MUL DIV MOD
%right NOT PLUS_PLUS MINUS_MINUS
%left DOT DDOT
%left LEFT_BRACKET RIGHT_BRACKET
%left LEFT_PARENTHESIS RIGHT_PARENTHESIS

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
                | BREAK SEMI_COLON                              {displayLog("stmt","BREAK SEMI_COLON");}
                | CONTINUE SEMI_COLON                           {displayLog("stmt","CONTINUE SEMI_COLON");}
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
                | PLUS_PLUS lvalue {
                                displayLog("term", "PLUS_PLUS lvalue");
                                if ($2 != nullptr && (($2->getType() == Alpha::SymbolType::LIBFUNC || $2->getType() == Alpha::SymbolType::USERFUNC)))
                                        symbolTable.registerSyntaxError(std::string("Operator ++  can not be used on function ") + $2->getName() , alpha_yylineno);
                        }
                | lvalue  PLUS_PLUS {
                                displayLog("term", "lvalue PLUS_PLUS");
                                if ($1 != nullptr && (($1->getType() == Alpha::SymbolType::LIBFUNC || $1->getType() == Alpha::SymbolType::USERFUNC)))
                                        symbolTable.registerSyntaxError(std::string("Operator ++  can not be used on function ") + $1->getName() , alpha_yylineno);
                        }
                | MINUS_MINUS lvalue {
                                displayLog("term", "MINUS_MINUS lvalue");
                                if ($2 != nullptr && (($2->getType() == Alpha::SymbolType::LIBFUNC || $2->getType() == Alpha::SymbolType::USERFUNC)))
                                        symbolTable.registerSyntaxError(std::string("Operator --  can not be used on function ") + $2->getName() , alpha_yylineno);
                        }
                | lvalue MINUS_MINUS {
                                displayLog("term", "lvalue MINUS_MINUS");
                                if ($1 != nullptr && (($1->getType() == Alpha::SymbolType::LIBFUNC || $1->getType() == Alpha::SymbolType::USERFUNC)))
                                        symbolTable.registerSyntaxError(std::string("Operator --  can not be used on function ") + $1->getName() , alpha_yylineno);
                        }
                | primary                                       {displayLog("term", "primary");}
                ;
                                //displaySyntaxrError("Symbol is already declared, but is not a variable.", alpha_yylineno);

assignexpr      : lvalue ASSIGN expr                            {displayLog("assignexpr", "lvalue ASSIGN expr");}
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
                                $$ = resultPair.second;
                        }
                | LOCAL ID {
                                displayLog("lvalue","LOCAL ID");
                                Alpha::SymbolTableEntry *entry = symbolTable.lookUpCurrentScope($2);
                                if (entry == nullptr && ((symbolTable.getCurrentScope() == symbolTable.GLOBAL_SCOPE_DEPTH
                                || symbolTable.lookUpGlobalScope($2)->getType() != Alpha::SymbolType::LIBFUNC)))
                                {
                                        // If here it is not a LIBFUNC, as initial lookUp was in global scope.
                                        symbolTable.insertVariable($2, alpha_yylineno);
                                        entry = symbolTable.lookUpVariable($2).second;
                                        if (entry == nullptr)
                                                throw std::runtime_error("Insertion of a variable failed after duplicate check");
                                }
                                else if (entry == nullptr && (entry = symbolTable.lookUpGlobalScope($2))->getType() == Alpha::SymbolType::LIBFUNC)
                                        symbolTable.registerSyntaxError(std::string($2) + " shadows library function", alpha_yylineno);
                                $$ = entry;
                        }
                | COLON_BLOCK ID {
                                displayLog("lvalue","COLON_BLOCK ID");
                                $$ = symbolTable.lookUpGlobalScope($2);
                                if ($$ == nullptr)
                                        symbolTable.registerSyntaxError(std::string("::") + std::string($2) + " not found in global scope", alpha_yylineno);
                        }
                | member                                        {displayLog("lvalue","member");}
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

methodcall      : DDOT ID LEFT_PARENTHESIS elist 
                  RIGHT_PARENTHESIS                             {displayLog("methodcall","DDOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
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
                        {symbolTable.decrementScope();}
                        RIGHT_BRACE
                        {displayLog("block", "LEFT_BRACE multi_stmt RIGHT_BRACE");}
                | LEFT_BRACE
                        RIGHT_BRACE
                        {displayLog("block", "LEFT_BRACE RIGHT_BRACE");}
                ;

funcdef         : FUNCTION
                        ID
                        {Alpha::Function::nameOfLastFunction = $2;}
                        LEFT_PARENTHESIS
                        idlist
                        RIGHT_PARENTHESIS 
                        {
                                Alpha::SymbolTableEntry *currentScopeEntry = symbolTable.lookUpCurrentScope(Alpha::Functon::nameOfLastFunction);
                                if (symbolTable.isLibraryFunction(Alpha::Function::nameOfLastFunction))
                                        symbolTable.registerSyntaxError(std::string("Redefinition of library function ") + Alpha::Function::nameOfLastFunction + " is prohibited", alpha_yylineno);
                                else if (currentScopeEntry && currentScopeEntry->getType() == Alpha::SymbolType::USERFUNC)
                                        symbolTable.registerSyntaxError(std::string("Function") + Alpha::Function:nameOfLastFunction + " is already defined in this scope. Can not redifine.", alpha_yylineno);
                                else if (currentScopeEntry) // We found a symbol, and it was a LIBFUNC nor a USERFUNC, thus it is a variable
                                        symbolTable.registerSyntaxError(std::string(Alpha::Function:nameOfLastFunction) + " is already defined as a variable.", alpha_yylineno);
                                else
                                {
                                        ///
                                        symbolTable.insertFunction(Alpha::Function:nameOfLastFunction, alpha_yylineno, Alpha::SymbolType::USERFUNC, Alpha::Function::idList) == OperationResult::DuplicateArgumentError)
                                        isFunctionBlock = true;
                                }
                        }
                        block
                        {
                                displayLog("funcdef", "FUNCTION ID LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block");
                        }
                | FUNCTION
                        LEFT_PARENTHESIS
                        idlist
                        RIGHT_PARENTHESIS
                        block
                        {displayLog("funcdef", "FUNCTION LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block");}
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
                        std::cout << "Rule2\n";
                        Alpha::Function::idList.push_back(std::string($1));
                        std::cout << $1 << std::endl;
                } COMMA cs_ids                                  {displayLog("cs_ids", "ID COMMA cs_ids");}
                ;

idlist          : cs_ids                                        {displayLog("idlist", "cs_ids");}
                |                                               {displayLog("idlist", "");}
                ;

ifstmt          : IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
                  stmt                                          {displayLog("ifstmt", "IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt");}
                | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS 
                  stmt ELSE  stmt                               {displayLog("ifstmt", "IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt");}
                ;

whilestmt       : WHILE LEFT_PARENTHESIS expr 
                  RIGHT_PARENTHESIS stmt                        {displayLog("whilestmt", "WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt");}
                ;

forstmt         : FOR LEFT_PARENTHESIS elist SEMI_COLON expr 
                  SEMI_COLON elist RIGHT_PARENTHESIS stmt       {displayLog("forstmt", "FOR LEFT_PARENTHESIS elist SEMI_CLON expr SEMI_CLON elist RIGHT_PARENTHESIS stmt");}
                ;

returnstmt      : RETURN SEMI_COLON                             {displayLog("returnstmt", "RETURN SEMI_COLON");}
                | RETURN expr SEMI_COLON                        {displayLog("returnstmt", "RETURN expr SEMI_COLON");}
                ;

indexedelem_list        : indexedelem                           {displayLog("indexedelem_list", "indexedelem");}
                        | indexedelem COMMA indexedelem_list    {displayLog("indexedelem_list", "indexedelem COMMA indexedelem_list");}
                        ;
