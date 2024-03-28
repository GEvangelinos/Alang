/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_ALPHA_YY_ALPHABISONPARSER_HPP_INCLUDED
# define YY_ALPHA_YY_ALPHABISONPARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef ALPHA_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ALPHA_YYDEBUG 1
#  else
#   define ALPHA_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ALPHA_YYDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ALPHA_YYDEBUG */
#if ALPHA_YYDEBUG
extern int alpha_yydebug;
#endif

/* Token kinds.  */
#ifndef ALPHA_YYTOKENTYPE
# define ALPHA_YYTOKENTYPE
  enum alpha_yytokentype
  {
    ALPHA_YYEMPTY = -2,
    ALPHA_YYEOF = 0,               /* "end of file"  */
    ALPHA_YYerror = 256,           /* error  */
    ALPHA_YYUNDEF = 257,           /* "invalid token"  */
    STRING_LITERAL = 258,          /* STRING_LITERAL  */
    ID = 259,                      /* ID  */
    INT_CONST = 260,               /* INT_CONST  */
    REAL_CONST = 261,              /* REAL_CONST  */
    IF = 262,                      /* IF  */
    ELSE = 263,                    /* ELSE  */
    WHILE = 264,                   /* WHILE  */
    FOR = 265,                     /* FOR  */
    FUNCTION = 266,                /* FUNCTION  */
    RETURN = 267,                  /* RETURN  */
    BREAK = 268,                   /* BREAK  */
    CONTINUE = 269,                /* CONTINUE  */
    AND = 270,                     /* AND  */
    NOT = 271,                     /* NOT  */
    OR = 272,                      /* OR  */
    LOCAL = 273,                   /* LOCAL  */
    TRUE = 274,                    /* TRUE  */
    FALSE = 275,                   /* FALSE  */
    NIL = 276,                     /* NIL  */
    ASSIGN = 277,                  /* ASSIGN  */
    PLUS = 278,                    /* PLUS  */
    MINUS = 279,                   /* MINUS  */
    MUL = 280,                     /* MUL  */
    DIV = 281,                     /* DIV  */
    MOD = 282,                     /* MOD  */
    EQUAL = 283,                   /* EQUAL  */
    NOT_EQUAL = 284,               /* NOT_EQUAL  */
    PLUS_PLUS = 285,               /* PLUS_PLUS  */
    MINUS_MINUS = 286,             /* MINUS_MINUS  */
    GREATER_THAN = 287,            /* GREATER_THAN  */
    LESS_THAN = 288,               /* LESS_THAN  */
    GREATER_THAN_OR_EQUAL = 289,   /* GREATER_THAN_OR_EQUAL  */
    LESS_THAN_OR_EQUAL = 290,      /* LESS_THAN_OR_EQUAL  */
    LEFT_BRACE = 291,              /* LEFT_BRACE  */
    RIGHT_BRACE = 292,             /* RIGHT_BRACE  */
    LEFT_BRACKET = 293,            /* LEFT_BRACKET  */
    RIGHT_BRACKET = 294,           /* RIGHT_BRACKET  */
    LEFT_PARENTHESIS = 295,        /* LEFT_PARENTHESIS  */
    RIGHT_PARENTHESIS = 296,       /* RIGHT_PARENTHESIS  */
    SEMI_COLON = 297,              /* SEMI_COLON  */
    COMMA = 298,                   /* COMMA  */
    COLON = 299,                   /* COLON  */
    COLON_BLOCK = 300,             /* COLON_BLOCK  */
    DOT = 301,                     /* DOT  */
    DDOT = 302                     /* DDOT  */
  };
  typedef enum alpha_yytokentype alpha_yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined ALPHA_YYSTYPE && ! defined ALPHA_YYSTYPE_IS_DECLARED
union ALPHA_YYSTYPE
{
#line 20 "alphaBisonParser.y"

        char *unionStringLiteral;
        char *unionId;
        long unionIntConst;
        double unionRealConst;
        void *union_lvalue;

#line 127 "alphaBisonParser.hpp"

};
typedef union ALPHA_YYSTYPE ALPHA_YYSTYPE;
# define ALPHA_YYSTYPE_IS_TRIVIAL 1
# define ALPHA_YYSTYPE_IS_DECLARED 1
#endif


extern ALPHA_YYSTYPE alpha_yylval;


int alpha_yyparse (void);


#endif /* !YY_ALPHA_YY_ALPHABISONPARSER_HPP_INCLUDED  */
