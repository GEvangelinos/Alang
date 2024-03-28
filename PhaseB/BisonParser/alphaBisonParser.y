%{
        #define INSIDE_BISON_FILE
        #include <string>
        #include <iostream>
        #include "../FlexScanner/alphaFlexScanner.hpp"
        #include "../alphaDefs.hpp"

        int yyerror(std::string message);
%}

%output "alphaBisonParser.cpp"
%define api.prefix {alpha_yy}
%defines
%expect 1


%start program

/* TODO: They say %union in BISON works only with POD types... Try to Use std::string or std::string * */
%union{
        char *unionStringLiteral;
        char *unionId;
        long unionIntConst;
        double unionRealConst;
        void *union_lvalue;
}

%token <unionStringLiteral> STRING_LITERAL
%token <unionId>ID
%token <unionIntConst> INT_CONST
%token <unionRealConst> REAL_CONST
%type <union_lvalue> lvalue



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
program         : /* Void */                                    {DISPLAY_LOG("program", "");}
                |  multi_stmt                                   {DISPLAY_LOG("program", "multi_stmt");}
                ;

stmt            : expr SEMI_COLON                               {DISPLAY_LOG("stmt","expr SEMI_COLON");}
                | ifstmt                                        {DISPLAY_LOG("stmt","ifstmt");}
                | whilestmt                                     {DISPLAY_LOG("stmt","whilestmt");}
                | forstmt                                       {DISPLAY_LOG("stmt","forstmt");}
                | returnstmt                                    {DISPLAY_LOG("stmt","returnstmt");}
                | BREAK SEMI_COLON                              {DISPLAY_LOG("stmt","BREAK SEMI_COLON");}
                | CONTINUE SEMI_COLON                           {DISPLAY_LOG("stmt","CONTINUE SEMI_COLON");}
                | block                                         {DISPLAY_LOG("stmt","block");}
                | funcdef                                       {DISPLAY_LOG("stmt","funcdef");}
                | SEMI_COLON                                    {DISPLAY_LOG("stmt","SEMI_COLON");}
                ;

multi_stmt      : stmt                                          {DISPLAY_LOG("multi_stmt", "stmt");}
                | stmt multi_stmt                               {DISPLAY_LOG("multi_stmt", "stmt multi_stmt");}
                ;

expr            : assignexpr                                    {DISPLAY_LOG("expr", "assignexpr");}
                | expr PLUS expr                                {DISPLAY_LOG("expr", "expr PLUS expr");}
                | expr MINUS expr                               {DISPLAY_LOG("expr", "expr MINUS expr");}
                | expr MUL expr                                 {DISPLAY_LOG("expr", "expr MUL expr");}
                | expr DIV expr                                 {DISPLAY_LOG("expr", "expr DIV expr");}
                | expr MOD expr                                 {DISPLAY_LOG("expr", "expr MOD expr");}
                | expr GREATER_THAN expr                        {DISPLAY_LOG("expr", "expr GREATER_THAN expr");}
                | expr GREATER_THAN_OR_EQUAL expr               {DISPLAY_LOG("expr", "expr GREATER_THAN_OR_EQUAL expr");}
                | expr LESS_THAN expr                           {DISPLAY_LOG("expr", "expr LESS_THAN expr");}
                | expr LESS_THAN_OR_EQUAL expr                  {DISPLAY_LOG("expr", "expr LESS_THAN_OR_EQUAL expr");}
                | expr EQUAL expr                               {DISPLAY_LOG("expr", "expr EQUAL expr ");}
                | expr NOT_EQUAL expr                           {DISPLAY_LOG("expr", "expr NOT_EQUAL expr");}
                | expr AND expr                                 {DISPLAY_LOG("expr", "expr AND expr");}
                | expr OR expr                                  {DISPLAY_LOG("expr", "expr OR expr");}
                | term                                          {DISPLAY_LOG("expr", "term");}
                ;

term            : LEFT_PARENTHESIS expr RIGHT_PARENTHESIS       {DISPLAY_LOG("term", "LEFT_PARENTHESIS expr RIGHT_PARENTHESIS");}
                | MINUS expr                                    {DISPLAY_LOG("term", "MINUS expr");}
                | NOT expr                                      {DISPLAY_LOG("term", "NOT expr");}
                | PLUS_PLUS lvalue                              {DISPLAY_LOG("term", "PLUS_PLUS lvalue");}
                | lvalue  PLUS_PLUS                             {DISPLAY_LOG("term", "lvalue PLUS_PLUS");}
                | MINUS_MINUS lvalue                            {DISPLAY_LOG("term", "MINUS_MINUS lvalue");}
                | lvalue MINUS_MINUS                            {DISPLAY_LOG("term", "lvalue MINUS_MINUS");}
                | primary                                       {DISPLAY_LOG("term", "primary");}
                ;

assignexpr      : lvalue ASSIGN expr                            {DISPLAY_LOG("assignexpr", "lvalue ASSIGN expr");}
                ;

primary         : lvalue                                        {DISPLAY_LOG("primary", "lvalue");}
                | call                                          {DISPLAY_LOG("primary", "call");}
                | objectdef                                     {DISPLAY_LOG("primary", "objectdef");}
                | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS    {DISPLAY_LOG("primary", "LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS");}
                | const                                         {DISPLAY_LOG("primary", "const");}
                ;

lvalue          : ID                                            /* TODO: PASS to symb_tbl possibly. */  {DISPLAY_LOG("lvalue","ID");}
                | LOCAL ID                                      /* TODO: PASS to symb_tbl possibly. */  {DISPLAY_LOG("lvalue","LOCAL ID");}
                | COLON_BLOCK ID                                /* TODO: PASS to symb_tbl possibly. */  {DISPLAY_LOG("lvalue","COLON_BLOCK ID");}
                | member                                        /* TODO: PASS to symb_tbl possibly. */  {DISPLAY_LOG("lvalue","member");}
                ;

member          : lvalue DOT ID                                 {DISPLAY_LOG("member","lvalue DOT ID");}
                | lvalue LEFT_BRACKET expr RIGHT_BRACKET        {DISPLAY_LOG("member","lvalue LEFT_BRACKET expr RIGHT_BRACKET");}
                | call DOT ID                                   {DISPLAY_LOG("member","call DOT ID");}
                | call LEFT_BRACKET expr RIGHT_BRACKET          {DISPLAY_LOG("member","CALL LEFT_BRACKET expr RIGHT_BRACKET");}
                ;

call            : call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {DISPLAY_LOG("call","call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
                | lvalue callsuffix                             {DISPLAY_LOG("call","lvalue callsuffix");}
                | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS 
                  LEFT_PARENTHESIS elist RIGHT_PARENTHESIS      {DISPLAY_LOG("call","LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
                ;

callsuffix      : normcall                                      {DISPLAY_LOG("callsuffix","normcall");}
                | methodcall                                    {DISPLAY_LOG("callsuffix","methodcall");}
                ;

normcall        : LEFT_PARENTHESIS elist RIGHT_PARENTHESIS      {DISPLAY_LOG("normcall","LEFT_PARENTHESIS elist RIGHT_PARENTHESIS")}
                ;

methodcall      : DDOT ID LEFT_PARENTHESIS elist 
                  RIGHT_PARENTHESIS                             {DISPLAY_LOG("methodcall","DDOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS")};
                ;

expr_list       : expr                                          {DISPLAY_LOG("expr_list", "expr");}
                | expr COMMA expr_list                          {DISPLAY_LOG("expr_list", "expr COMMA expr_list");}
                ;

elist           : expr_list                                     {DISPLAY_LOG("elist", "expr_list");}
                | /* Void */                                    {DISPLAY_LOG("elist", "");}
                ;

objectdef       : LEFT_BRACKET elist RIGHT_BRACKET              {DISPLAY_LOG("objectdef", "LEFT_BRACKET elist RIGHT_BRACKET");}
                | LEFT_BRACKET indexed RIGHT_BRACKET            {DISPLAY_LOG("objectdef", "LEFT_BRACKET indexed RIGHT_BRACKET");}
                ;

indexed         : indexedelem_list                              {DISPLAY_LOG("indexed", "indexedelem_list");}     
                ; /* | EMPTY (Removed to avoid conflict.. else I could use %expect 2. */

indexedelem     : LEFT_BRACE expr COLON expr RIGHT_BRACE        {DISPLAY_LOG("indexedelem", "LEFT_BRACE expr COLON expr RIGHT_BRACE");}       
                ;

block           : LEFT_BRACE multi_stmt RIGHT_BRACE             {DISPLAY_LOG("block", "LEFT_BRACE multi_stmt RIGHT_BRACE");}
                ;

funcdef         : FUNCTION ID LEFT_PARENTHESIS idlist 
                  RIGHT_PARENTHESIS block                       {DISPLAY_LOG("funcdef", "FUNCTION ID LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block");}
                | FUNCTION LEFT_PARENTHESIS idlist
                  RIGHT_PARENTHESIS block                       {DISPLAY_LOG("funcdef", "FUNCTION LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block");}
                ;

const           : INT_CONST                                     {DISPLAY_LOG("const", "INT_CONST");}
                | REAL_CONST                                    {DISPLAY_LOG("const", "REAL_CONST");}
                | STRING_LITERAL                                {DISPLAY_LOG("const", "STRING_LITERAL");}
                | NIL                                           {DISPLAY_LOG("const", "NIL");}
                | TRUE                                          {DISPLAY_LOG("const", "TRUE");}
                | FALSE                                         {DISPLAY_LOG("const", "FALSE");}
                ;

idlist          : ID                                            {DISPLAY_LOG("idlist", "ID");}
                | ID COMMA idlist                               {DISPLAY_LOG("idlist", "ID COMMA idlist");}
                ;

ifstmt          : IF LEFT_PARENTHESIS expr 
                  RIGHT_PARENTHESIS stmt                        {DISPLAY_LOG("ifstmt", "IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt");}
                | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS 
                  stmt ELSE  stmt                               {DISPLAY_LOG("ifstmt", "IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt");}
                ;

whilestmt       : WHILE LEFT_PARENTHESIS expr 
                  RIGHT_PARENTHESIS stmt                        {DISPLAY_LOG("whilestmt", "WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt");}
                ;

forstmt         : FOR LEFT_PARENTHESIS elist SEMI_COLON expr 
                  SEMI_COLON elist RIGHT_PARENTHESIS stmt       {DISPLAY_LOG("forstmt", "FOR LEFT_PARENTHESIS elist SEMI_CLON expr SEMI_CLON elist RIGHT_PARENTHESIS stmt");}
                ;

returnstmt      : RETURN SEMI_COLON                             {DISPLAY_LOG("returnstmt", "RETURN SEMI_COLON");}
                | RETURN expr SEMI_COLON                        {DISPLAY_LOG("returnstmt", "RETURN expr SEMI_COLON");}
                ;

indexedelem_list        : indexedelem                           {DISPLAY_LOG("indexedelem_list", "indexedelem");}
                        | indexedelem COMMA indexedelem_list    {DISPLAY_LOG("indexedelem_list", "indexedelem COMMA indexedelem_list");}
                        ;

