%{
        #include <string>
        #include <iostream>
        extern int 
%}

%output "alphaBisonParser.cpp"
%define parse.error verbose



%start program

/* TODO: They say %union in BISON works only with POD types... Try to Use std::string or std::string * */
%union{
        char *unionStringLiteral;
        char *unionId;
        int unionIntConst;
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
program         : /* Void */
                |  multi_stmt
                ;

stmt            : expr SEMI_COLON
                | ifstmt
                | whilestmt
                | forstmt
                | returnstmt
                | BREAK SEMI_COLON
                | CONTINUE SEMI_COLON
                | block
                | funcdef
                | SEMI_COLON
                ;

multi_stmt      : stmt
                | stmt multi_stmt
                ;

expr            : assignexpr
                | expr PLUS expr
                | expr MINUS expr
                | expr MUL expr
                | expr DIV expr
                | expr MOD expr
                | expr GREATER_THAN expr
                | expr GREATER_THAN_OR_EQUAL expr
                | expr LESS_THAN expr
                | expr LESS_THAN_OR_EQUAL expr
                | expr EQUAL expr
                | expr NOT_EQUAL expr
                | expr AND expr
                | expr OR expr
                | term
                ;

term            : LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
                | MINUS expr
                | NOT expr
                | PLUS_PLUS lvalue
                | lvalue  PLUS_PLUS
                | MINUS_MINUS lvalue
                | lvalue MINUS_MINUS
                | primary
                ;

assignexpr      : lvalue ASSIGN expr
                ;

primary         : lvalue
                | call
                | objectdef
                | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS
                | const
                ;

lvalue          : ID {std::cout << "HELLO\n";}
                | LOCAL ID {std::cout << "Hello2\n";}
                | COLON_BLOCK ID {std::cout << "Hello3\n";}
                | member {std::cout << "Hello4\n";}
                ;

member          : lvalue DOT ID
                | lvalue LEFT_BRACKET expr RIGHT_BRACKET
                | call DOT ID
                | call LEFT_BRACKET expr RIGHT_BRACKET
                ;

call            : call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
                | lvalue callsuffix
                | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
                ;

callsuffix      : normcall
                | methodcall
                ;

normcall        : LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
                ;

methodcall      : DDOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
                ;

expr_list       : expr
                | expr COMMA expr_list
                ;

elist           : expr_list
                | /* Void */
                ;

objectdef       : LEFT_BRACKET elist RIGHT_BRACKET
                | LEFT_BRACKET indexed RIGHT_BRACKET
                ;

indexed         : indexedelem_list
                ;

indexedelem     : LEFT_BRACE expr COLON expr RIGHT_BRACE
                ;

block           : LEFT_BRACE multi_stmt RIGHT_BRACE
                ;

funcdef         : FUNCTION ID LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block
                | FUNCTION LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block
                ;

const           : INT_CONST
                | REAL_CONST
                | STRING_LITERAL
                | NIL
                | TRUE
                | FALSE
                ;

idlist          : ID
                | ID COMMA idlist
                ;

ifstmt          : IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt
                | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE  stmt
                ;

whilestmt       : WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt
                ;

forstmt         : FOR LEFT_PARENTHESIS elist SEMI_COLON expr SEMI_COLON elist RIGHT_PARENTHESIS stmt
                ;

returnstmt      : RETURN SEMI_COLON
                | RETURN expr SEMI_COLON
                ;

indexedelem_list        : indexedelem
                        | indexedelem COMMA indexedelem_list
                        ;

