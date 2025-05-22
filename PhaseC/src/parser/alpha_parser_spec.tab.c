/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */
#line 2 "alpha_parser_spec.y"

         // IWYU pragma: no_include <features.h>
         // IWYU pragma: no_include <stdio.h>
         // IWYU pragma: no_include <stdlib.h>
         // IWYU pragma: no_include <string.h>
        #include <string>                             // for basic_string, string
        #include "parser/alpha_trace_logger.hpp"            // for display_trace
        #include "parser/alpha_parser_context.hpp"    // for ParseCtx
        #include "scanner/alpha_scanner_context.hpp"  // for LexerCtx
        #include "alpha_parser_prologue_code.hpp" // THIS MUST STAY in parser's.cpp not parser's .hpp
        using AOP =  Alpha::IOPCode;

#line 81 "alpha_parser_spec.tab.c"
/* Substitute the type names.  */
#define YYSTYPE         ALPHA_YYSTYPE
#define YYLTYPE         ALPHA_YYLTYPE
/* Substitute the variable and function names.  */
#define yyparse         alpha_yyparse
#define yylex           alpha_yylex
#define yyerror         alpha_yyerror
#define yydebug         alpha_yydebug
#define yynerrs         alpha_yynerrs
#define yylval          alpha_yylval
#define yychar          alpha_yychar
#define yylloc          alpha_yylloc


# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif


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
/* "%code requires" blocks.  */
#line 16 "alpha_parser_spec.y"

        #include "core/alpha_error.hpp"               // for ErrorTracker
        #include "core/alpha_location.hpp"            // for Location, LocationTracker
        #include "parser/alpha_semantic_manager.hpp"  // for block__lbrace, funcArgs...
        #include "parser/alpha_semantic_builder.hpp"  // for block__lbrace, funcArgs...
        #include "parser/alpha_parser_context.hpp"    // for ParseCtx
        #include "parser/alpha_symbol_table.hpp"      // for Symbol, SymbolTable
        #include "scanner/alpha_scanner_context.hpp"  // for LexerCtx

#line 144 "alpha_parser_spec.tab.c"

/* Token kinds.  */
#ifndef ALPHA_YYTOKENTYPE
# define ALPHA_YYTOKENTYPE
  enum alpha_yytokentype
  {
    ALPHA_YYEMPTY = -2,
    ALPHA_YYEOF = 0,               /* "end of file"  */
    ALPHA_YYerror = 256,           /* error  */
    ALPHA_YYUNDEF = 257,           /* "invalid token"  */
    STRING = 258,                  /* "`string-literal`"  */
    ID = 259,                      /* "`identifier`"  */
    INT = 260,                     /* "`integer-constant`"  */
    REAL = 261,                    /* "`real-constant`"  */
    IF = 262,                      /* "keyword `if`"  */
    ELSE = 263,                    /* "keyword `else`"  */
    WHILE = 264,                   /* "keyword `while`"  */
    FOR = 265,                     /* "keyword `for`"  */
    CONTINUE = 266,                /* "keyword `continue`"  */
    BREAK = 267,                   /* "keyword `break`"  */
    FUNCTION = 268,                /* "keyword `function`"  */
    RETURN = 269,                  /* "keyword `return`"  */
    NOT = 270,                     /* "keyword `not`"  */
    AND = 271,                     /* "keyword `and`"  */
    OR = 272,                      /* "keyword `or`"  */
    LOCAL = 273,                   /* "keyword `local`"  */
    NIL = 274,                     /* "keyword `nil`"  */
    TRUE = 275,                    /* "keyword `true`"  */
    FALSE = 276,                   /* "keyword `false`"  */
    ASSIGN = 277,                  /* "assignment operator ="  */
    PLUS = 278,                    /* "+"  */
    MINUS = 279,                   /* "-"  */
    MUL = 280,                     /* "*"  */
    DIV = 281,                     /* "/"  */
    MOD = 282,                     /* "%"  */
    LT = 283,                      /* ">"  */
    GT = 284,                      /* "<"  */
    GTE = 285,                     /* ">="  */
    LTE = 286,                     /* "<="  */
    EQ = 287,                      /* "=="  */
    NEQ = 288,                     /* "!="  */
    DEC = 289,                     /* "decrement operator `--`"  */
    INC = 290,                     /* "increment operator `++`"  */
    LEFT_BRACE = 291,              /* "{"  */
    RIGHT_BRACE = 292,             /* "}"  */
    LEFT_BRACKET = 293,            /* "["  */
    RIGHT_BRACKET = 294,           /* "]"  */
    LEFT_PAREN = 295,              /* "("  */
    RIGHT_PAREN = 296,             /* ")"  */
    SEMICOLON = 297,               /* ";"  */
    COMMA = 298,                   /* ","  */
    DOT = 299,                     /* "."  */
    METHOD_CALL = 300,             /* "method-call operator .."  */
    COLON = 301,                   /* ":"  */
    GLOBAL = 302,                  /* "global operator ::"  */
    UMINUS = 303,                  /* UMINUS  */
    THEN = 304                     /* THEN  */
  };
  typedef enum alpha_yytokentype alpha_yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined ALPHA_YYSTYPE && ! defined ALPHA_YYSTYPE_IS_DECLARED
union ALPHA_YYSTYPE
{
#line 47 "alpha_parser_spec.y"

        char *cstring;
        bool const_bool;
        long const_int;
        double const_real;
        const  Alpha::Function *const_function_symbol_ptr;
        Alpha::Expr *expr_ptr;
        Alpha::ExprList *expr_list_ptr;
        Alpha::ExprPair *expr_pair_ptr;
        Alpha::DictList *dict_list_ptr;

        Alpha::Location location;
        Alpha::BlockLocation block_location;

#line 225 "alpha_parser_spec.tab.c"

};
typedef union ALPHA_YYSTYPE ALPHA_YYSTYPE;
# define ALPHA_YYSTYPE_IS_TRIVIAL 1
# define ALPHA_YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
typedef Alpha::Location ALPHA_YYLTYPE;


extern ALPHA_YYSTYPE alpha_yylval;
extern ALPHA_YYLTYPE alpha_yylloc;

int alpha_yyparse (Alpha::LocationTracker &location_tracker, Alpha::ErrorTracker &error_tracker, Alpha::LexerCtx &lexer_ctx, Alpha::SemanticManager &sm, Alpha::SemanticBuilder &sb);



/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_STRING = 3,                     /* "`string-literal`"  */
  YYSYMBOL_ID = 4,                         /* "`identifier`"  */
  YYSYMBOL_INT = 5,                        /* "`integer-constant`"  */
  YYSYMBOL_REAL = 6,                       /* "`real-constant`"  */
  YYSYMBOL_IF = 7,                         /* "keyword `if`"  */
  YYSYMBOL_ELSE = 8,                       /* "keyword `else`"  */
  YYSYMBOL_WHILE = 9,                      /* "keyword `while`"  */
  YYSYMBOL_FOR = 10,                       /* "keyword `for`"  */
  YYSYMBOL_CONTINUE = 11,                  /* "keyword `continue`"  */
  YYSYMBOL_BREAK = 12,                     /* "keyword `break`"  */
  YYSYMBOL_FUNCTION = 13,                  /* "keyword `function`"  */
  YYSYMBOL_RETURN = 14,                    /* "keyword `return`"  */
  YYSYMBOL_NOT = 15,                       /* "keyword `not`"  */
  YYSYMBOL_AND = 16,                       /* "keyword `and`"  */
  YYSYMBOL_OR = 17,                        /* "keyword `or`"  */
  YYSYMBOL_LOCAL = 18,                     /* "keyword `local`"  */
  YYSYMBOL_NIL = 19,                       /* "keyword `nil`"  */
  YYSYMBOL_TRUE = 20,                      /* "keyword `true`"  */
  YYSYMBOL_FALSE = 21,                     /* "keyword `false`"  */
  YYSYMBOL_ASSIGN = 22,                    /* "assignment operator ="  */
  YYSYMBOL_PLUS = 23,                      /* "+"  */
  YYSYMBOL_MINUS = 24,                     /* "-"  */
  YYSYMBOL_MUL = 25,                       /* "*"  */
  YYSYMBOL_DIV = 26,                       /* "/"  */
  YYSYMBOL_MOD = 27,                       /* "%"  */
  YYSYMBOL_LT = 28,                        /* ">"  */
  YYSYMBOL_GT = 29,                        /* "<"  */
  YYSYMBOL_GTE = 30,                       /* ">="  */
  YYSYMBOL_LTE = 31,                       /* "<="  */
  YYSYMBOL_EQ = 32,                        /* "=="  */
  YYSYMBOL_NEQ = 33,                       /* "!="  */
  YYSYMBOL_DEC = 34,                       /* "decrement operator `--`"  */
  YYSYMBOL_INC = 35,                       /* "increment operator `++`"  */
  YYSYMBOL_LEFT_BRACE = 36,                /* "{"  */
  YYSYMBOL_RIGHT_BRACE = 37,               /* "}"  */
  YYSYMBOL_LEFT_BRACKET = 38,              /* "["  */
  YYSYMBOL_RIGHT_BRACKET = 39,             /* "]"  */
  YYSYMBOL_LEFT_PAREN = 40,                /* "("  */
  YYSYMBOL_RIGHT_PAREN = 41,               /* ")"  */
  YYSYMBOL_SEMICOLON = 42,                 /* ";"  */
  YYSYMBOL_COMMA = 43,                     /* ","  */
  YYSYMBOL_DOT = 44,                       /* "."  */
  YYSYMBOL_METHOD_CALL = 45,               /* "method-call operator .."  */
  YYSYMBOL_COLON = 46,                     /* ":"  */
  YYSYMBOL_GLOBAL = 47,                    /* "global operator ::"  */
  YYSYMBOL_UMINUS = 48,                    /* UMINUS  */
  YYSYMBOL_THEN = 49,                      /* THEN  */
  YYSYMBOL_YYACCEPT = 50,                  /* $accept  */
  YYSYMBOL_program = 51,                   /* program  */
  YYSYMBOL_multiStmt = 52,                 /* multiStmt  */
  YYSYMBOL_53_1 = 53,                      /* $@1  */
  YYSYMBOL_stmt = 54,                      /* stmt  */
  YYSYMBOL_loopCtrlStmt = 55,              /* loopCtrlStmt  */
  YYSYMBOL_expr = 56,                      /* expr  */
  YYSYMBOL_57_2 = 57,                      /* $@2  */
  YYSYMBOL_orHook = 58,                    /* orHook  */
  YYSYMBOL_andHook = 59,                   /* andHook  */
  YYSYMBOL_term = 60,                      /* term  */
  YYSYMBOL_assignExpr = 61,                /* assignExpr  */
  YYSYMBOL_primary = 62,                   /* primary  */
  YYSYMBOL_lvalue = 63,                    /* lvalue  */
  YYSYMBOL_tableItem = 64,                 /* tableItem  */
  YYSYMBOL_member = 65,                    /* member  */
  YYSYMBOL_methodCallId = 66,              /* methodCallId  */
  YYSYMBOL_call = 67,                      /* call  */
  YYSYMBOL_exprList = 68,                  /* exprList  */
  YYSYMBOL_elist = 69,                     /* elist  */
  YYSYMBOL_tableList = 70,                 /* tableList  */
  YYSYMBOL_tableDict = 71,                 /* tableDict  */
  YYSYMBOL_objectDef = 72,                 /* objectDef  */
  YYSYMBOL_indexed = 73,                   /* indexed  */
  YYSYMBOL_indexedElemList = 74,           /* indexedElemList  */
  YYSYMBOL_indexedElem = 75,               /* indexedElem  */
  YYSYMBOL_blockBegin = 76,                /* blockBegin  */
  YYSYMBOL_blockEnd = 77,                  /* blockEnd  */
  YYSYMBOL_block = 78,                     /* block  */
  YYSYMBOL_funcPrefix = 79,                /* funcPrefix  */
  YYSYMBOL_funcArgs = 80,                  /* funcArgs  */
  YYSYMBOL_81_3 = 81,                      /* $@3  */
  YYSYMBOL_funcArgList = 82,               /* funcArgList  */
  YYSYMBOL_funcSignature = 83,             /* funcSignature  */
  YYSYMBOL_funcDef = 84,                   /* funcDef  */
  YYSYMBOL_const = 85,                     /* const  */
  YYSYMBOL_ifPrefix = 86,                  /* ifPrefix  */
  YYSYMBOL_elsePrefix = 87,                /* elsePrefix  */
  YYSYMBOL_ifStmt = 88,                    /* ifStmt  */
  YYSYMBOL_whileHeader = 89,               /* whileHeader  */
  YYSYMBOL_whileStmt = 90,                 /* whileStmt  */
  YYSYMBOL_91_4 = 91,                      /* $@4  */
  YYSYMBOL_forHeader = 92,                 /* forHeader  */
  YYSYMBOL_forStmt = 93,                   /* forStmt  */
  YYSYMBOL_94_5 = 94,                      /* $@5  */
  YYSYMBOL_funcCtrlStmt = 95,              /* funcCtrlStmt  */
  YYSYMBOL_returnStmt = 96                 /* returnStmt  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
# define YYCOPY_NEEDED 1
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined ALPHA_YYLTYPE_IS_TRIVIAL && ALPHA_YYLTYPE_IS_TRIVIAL \
             && defined ALPHA_YYSTYPE_IS_TRIVIAL && ALPHA_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  80
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   594

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  47
/* YYNRULES -- Number of rules.  */
#define YYNRULES  111
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  191

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   304


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49
};

#if ALPHA_YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   186,   186,   187,   191,   194,   193,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     215,   216,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   240,   239,   249,   253,   257,
     261,   262,   263,   264,   265,   266,   267,   268,   272,   277,
     278,   279,   280,   282,   287,   288,   289,   290,   294,   296,
     301,   302,   303,   307,   313,   315,   317,   319,   324,   326,
     331,   332,   336,   341,   346,   347,   351,   355,   357,   362,
     367,   375,   383,   385,   391,   392,   396,   397,   397,   401,
     402,   406,   411,   420,   421,   422,   423,   424,   425,   433,
     438,   442,   443,   447,   452,   451,   458,   463,   462,   469,
     473,   474
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "\"`string-literal`\"",
  "\"`identifier`\"", "\"`integer-constant`\"", "\"`real-constant`\"",
  "\"keyword `if`\"", "\"keyword `else`\"", "\"keyword `while`\"",
  "\"keyword `for`\"", "\"keyword `continue`\"", "\"keyword `break`\"",
  "\"keyword `function`\"", "\"keyword `return`\"", "\"keyword `not`\"",
  "\"keyword `and`\"", "\"keyword `or`\"", "\"keyword `local`\"",
  "\"keyword `nil`\"", "\"keyword `true`\"", "\"keyword `false`\"",
  "\"assignment operator =\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\"%\"",
  "\">\"", "\"<\"", "\">=\"", "\"<=\"", "\"==\"", "\"!=\"",
  "\"decrement operator `--`\"", "\"increment operator `++`\"", "\"{\"",
  "\"}\"", "\"[\"", "\"]\"", "\"(\"", "\")\"", "\";\"", "\",\"", "\".\"",
  "\"method-call operator ..\"", "\":\"", "\"global operator ::\"",
  "UMINUS", "THEN", "$accept", "program", "multiStmt", "$@1", "stmt",
  "loopCtrlStmt", "expr", "$@2", "orHook", "andHook", "term", "assignExpr",
  "primary", "lvalue", "tableItem", "member", "methodCallId", "call",
  "exprList", "elist", "tableList", "tableDict", "objectDef", "indexed",
  "indexedElemList", "indexedElem", "blockBegin", "blockEnd", "block",
  "funcPrefix", "funcArgs", "$@3", "funcArgList", "funcSignature",
  "funcDef", "const", "ifPrefix", "elsePrefix", "ifStmt", "whileHeader",
  "whileStmt", "$@4", "forHeader", "forStmt", "$@5", "funcCtrlStmt",
  "returnStmt", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-52)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-112)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     168,    36,   -52,   -52,   -52,   -52,   -28,    -7,    -4,   -52,
     -52,    18,   -52,   298,    33,   -52,   -52,   -52,   298,    16,
      16,   -52,   103,    11,   -52,    34,    39,   -52,     6,    -2,
     368,   -52,   -52,   -52,   111,   -52,   -52,   -19,   -52,   -52,
     -52,   213,   -52,    12,    21,   -52,   -52,   258,   -52,   -52,
     -52,   -52,   -52,   298,    19,   -52,   -52,   -52,   -52,   298,
     298,   298,   -52,    37,   -52,    37,    52,    24,   -19,    24,
     298,   347,   -52,    27,    28,   -52,    31,   428,    35,   -52,
     -52,   258,   -52,   -52,   298,   298,   298,   298,   298,   298,
     298,   298,   298,   298,   298,   -52,    62,   298,   -52,   -52,
     298,   298,    76,    78,    43,   298,   298,    91,   -52,    59,
     -52,     7,   -52,   -52,    93,   258,   258,   388,   -52,   447,
     466,    57,    61,   323,   298,   -52,   -52,    67,   -52,    64,
     -52,   298,   -18,   -18,   -52,   -52,   -52,   170,   170,   170,
     170,   550,   550,   -52,   539,   485,    69,   -52,   -52,   298,
     503,    70,   -52,   -52,    77,   -52,    71,   -52,   258,   -52,
     -52,   -52,   -52,   298,    64,   298,   -52,   -52,   298,   561,
     298,   -52,   -52,    72,   -52,   -52,    82,   -52,   -52,   408,
     521,    85,   539,   -52,   115,   298,   -52,   -52,   -52,    87,
     -52
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,    98,    54,    96,    97,     0,     0,     0,    21,
      20,    84,   109,     0,     0,    93,    94,    95,     0,     0,
       0,    80,    70,     0,    15,     0,     0,     3,     5,     0,
      35,    37,    22,    47,    49,    60,    57,    50,    74,    75,
      51,     0,    13,     0,     0,    14,    53,     0,     8,   104,
       9,   107,    10,   110,     0,    19,    18,    17,    16,     0,
       0,    70,    85,    42,    55,    41,     0,    45,     0,    43,
       0,    68,    71,     0,     0,    76,    77,    35,     0,    56,
       1,     0,    12,    39,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     7,     0,     0,    46,    44,
       0,    70,     0,     0,     0,     0,    70,     0,    81,     0,
      83,     0,    91,    92,   101,     0,     0,    35,    11,    35,
      35,     0,     0,    35,     0,    72,    73,     0,    40,    52,
       6,     0,    23,    24,    25,    26,    27,    30,    28,    29,
      31,    32,    33,    38,    48,    35,     0,    58,    63,    70,
      35,     0,    61,    82,    86,    89,     0,   100,     0,   105,
     108,    99,   103,     0,     0,     0,    69,    78,    70,    34,
       0,    59,    65,     0,    62,    64,     0,    90,   102,    35,
      35,     0,    36,    66,     0,    70,    79,    67,    88,     0,
     106
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -52,   -52,   -39,   -52,   -44,   -52,     0,   -52,   -52,   -52,
     -52,   -52,   -52,   -15,   -52,   -52,   -52,     8,     5,   -51,
     -52,   -52,   -52,   -52,     3,   -52,   -52,    23,    92,   -52,
     -49,   -52,   -52,   -52,   -22,   -52,   -52,   -52,   -52,   -52,
     -52,   -52,   -52,   -52,   -52,   -52,   -52
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    26,    27,    81,    28,    29,    71,    96,   170,   131,
      31,    32,    33,    34,    35,    36,   104,    37,    72,    73,
      38,    39,    40,    74,    75,    76,    41,   110,    42,    43,
     156,   176,   112,    44,    45,    46,    47,   158,    48,    49,
      50,   115,    51,    52,   116,    53,    54
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,    78,   109,   114,    67,    69,    -4,    86,    87,    88,
     121,   154,    59,    63,     2,     3,     4,     5,    65,   105,
       3,   106,    62,    77,    11,   107,    13,    68,    68,    14,
      15,    16,    17,    60,    14,    18,    61,    64,    79,    80,
      82,    30,   130,    -4,   122,    19,    20,    30,   155,    22,
     146,    23,   111,   117,   -35,   151,    66,    21,    25,   119,
     120,   118,   100,    25,   101,    11,   125,   126,   102,   103,
     123,   159,   160,    55,   127,    56,   129,    57,    58,   143,
     147,    30,   148,   149,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   152,   108,   144,   173,   163,
     145,   157,   164,    70,   168,   150,     2,     3,     4,     5,
     172,   175,   177,   183,   178,    30,    30,   181,    13,   154,
     -87,    14,    15,    16,    17,   184,   187,    18,   190,   166,
     167,   169,   153,    97,   189,   188,   113,    19,    20,    70,
       0,    22,     0,    23,     0,    98,    99,     0,     0,   100,
      25,   101,     0,     0,     0,   102,   103,     0,    30,     0,
       0,     0,     0,   179,     0,   180,     0,     0,    -2,     1,
     182,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,    12,    13,     0,     0,    14,    15,    16,    17,
       0,     0,    18,    84,    85,    86,    87,    88,  -112,  -112,
    -112,  -112,    19,    20,    21,     0,    22,     0,    23,     0,
      24,     0,     0,     0,     1,    25,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,    12,    13,     0,
       0,    14,    15,    16,    17,     0,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,    20,    21,
     108,    22,     0,    23,     0,    24,     0,     0,     0,     1,
      25,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,    12,    13,     0,     0,    14,    15,    16,    17,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,    20,    21,     0,    22,     0,    23,     0,
      24,     2,     3,     4,     5,    25,     0,     0,     0,     0,
       0,     0,     0,    13,     0,     0,    14,    15,    16,    17,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,    20,     0,     0,    22,     0,    23,    83,
       0,     0,     0,     0,     0,    25,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,     0,     0,
       0,     0,     0,    83,   -35,     0,     0,     0,     0,   165,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,    83,     0,     0,     0,     0,     0,
     124,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,     0,    83,     0,     0,     0,     0,     0,
      95,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,     0,    83,     0,     0,     0,     0,     0,
    -111,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,     0,    83,     0,     0,     0,     0,     0,
     185,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    83,     0,     0,     0,     0,     0,   128,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,    83,     0,     0,     0,     0,     0,   161,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    83,     0,     0,     0,     0,     0,   162,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    83,
       0,     0,     0,     0,   171,     0,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    83,     0,     0,
       0,     0,   174,     0,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    83,   -35,     0,   186,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    84,    85,    86,    87,    88,    89,    90,
      91,    92,  -112,  -112,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94
};

static const yytype_int16 yycheck[] =
{
       0,    23,    41,    47,    19,    20,     0,    25,    26,    27,
      61,     4,    40,    13,     3,     4,     5,     6,    18,    38,
       4,    40,     4,    23,    13,    44,    15,    19,    20,    18,
      19,    20,    21,    40,    18,    24,    40,     4,     4,     0,
      42,    41,    81,    37,    66,    34,    35,    47,    41,    38,
     101,    40,    40,    53,    17,   106,    40,    36,    47,    59,
      60,    42,    38,    47,    40,    13,    39,    39,    44,    45,
      70,   115,   116,    37,    43,    39,    41,    41,    42,    17,
       4,    81,     4,    40,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     4,    37,    97,   149,    42,
     100,     8,    41,    36,    40,   105,     3,     4,     5,     6,
      41,    41,    41,    41,   158,   115,   116,   168,    15,     4,
      43,    18,    19,    20,    21,    43,    41,    24,    41,   124,
     127,   131,   109,    22,   185,   184,    44,    34,    35,    36,
      -1,    38,    -1,    40,    -1,    34,    35,    -1,    -1,    38,
      47,    40,    -1,    -1,    -1,    44,    45,    -1,   158,    -1,
      -1,    -1,    -1,   163,    -1,   165,    -1,    -1,     0,     1,
     170,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    14,    15,    -1,    -1,    18,    19,    20,    21,
      -1,    -1,    24,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    34,    35,    36,    -1,    38,    -1,    40,    -1,
      42,    -1,    -1,    -1,     1,    47,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    14,    15,    -1,
      -1,    18,    19,    20,    21,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    35,    36,
      37,    38,    -1,    40,    -1,    42,    -1,    -1,    -1,     1,
      47,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    14,    15,    -1,    -1,    18,    19,    20,    21,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    34,    35,    36,    -1,    38,    -1,    40,    -1,
      42,     3,     4,     5,     6,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    -1,    -1,    18,    19,    20,    21,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    34,    35,    -1,    -1,    38,    -1,    40,    16,
      -1,    -1,    -1,    -1,    -1,    47,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    16,    17,    -1,    -1,    -1,    -1,    46,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    -1,    -1,    16,    -1,    -1,    -1,    -1,    -1,
      43,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    -1,    16,    -1,    -1,    -1,    -1,    -1,
      42,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    -1,    16,    -1,    -1,    -1,    -1,    -1,
      42,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    -1,    16,    -1,    -1,    -1,    -1,    -1,
      42,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    16,    -1,    -1,    -1,    -1,    -1,    41,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    16,    -1,    -1,    -1,    -1,    -1,    41,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    16,    -1,    -1,    -1,    -1,    -1,    41,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    16,
      -1,    -1,    -1,    -1,    39,    -1,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    16,    -1,    -1,
      -1,    -1,    39,    -1,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    16,    17,    -1,    37,    -1,
      -1,    -1,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     9,    10,    11,
      12,    13,    14,    15,    18,    19,    20,    21,    24,    34,
      35,    36,    38,    40,    42,    47,    51,    52,    54,    55,
      56,    60,    61,    62,    63,    64,    65,    67,    70,    71,
      72,    76,    78,    79,    83,    84,    85,    86,    88,    89,
      90,    92,    93,    95,    96,    37,    39,    41,    42,    40,
      40,    40,     4,    56,     4,    56,    40,    63,    67,    63,
      36,    56,    68,    69,    73,    74,    75,    56,    84,     4,
       0,    53,    42,    16,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    42,    57,    22,    34,    35,
      38,    40,    44,    45,    66,    38,    40,    44,    37,    52,
      77,    40,    82,    78,    54,    91,    94,    56,    42,    56,
      56,    69,    84,    56,    43,    39,    39,    43,    41,    41,
      52,    59,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    17,    56,    56,    69,     4,     4,    40,
      56,    69,     4,    77,     4,    41,    80,     8,    87,    54,
      54,    41,    41,    42,    41,    46,    68,    74,    40,    56,
      58,    39,    41,    69,    39,    41,    81,    41,    54,    56,
      56,    69,    56,    41,    43,    42,    37,    41,    80,    69,
      41
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    50,    51,    51,    52,    53,    52,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      55,    55,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    57,    56,    56,    58,    59,
      60,    60,    60,    60,    60,    60,    60,    60,    61,    62,
      62,    62,    62,    62,    63,    63,    63,    63,    64,    64,
      65,    65,    65,    66,    67,    67,    67,    67,    68,    68,
      69,    69,    70,    71,    72,    72,    73,    74,    74,    75,
      76,    77,    78,    78,    79,    79,    80,    81,    80,    82,
      82,    83,    84,    85,    85,    85,    85,    85,    85,    86,
      87,    88,    88,    89,    91,    90,    92,    94,    93,    95,
      96,    96
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     1,     0,     3,     2,     1,     1,
       1,     2,     2,     1,     1,     1,     2,     2,     2,     2,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     4,     0,     5,     1,     0,     0,
       3,     2,     2,     2,     2,     2,     2,     1,     3,     1,
       1,     1,     3,     1,     1,     2,     2,     1,     3,     4,
       1,     3,     4,     2,     4,     4,     5,     6,     1,     3,
       0,     1,     3,     3,     1,     1,     1,     1,     3,     5,
       1,     1,     3,     2,     1,     2,     1,     0,     4,     2,
       3,     2,     2,     1,     1,     1,     1,     1,     1,     4,
       1,     2,     4,     4,     0,     3,     8,     0,     3,     1,
       1,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = ALPHA_YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == ALPHA_YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        YY_LAC_DISCARD ("YYBACKUP");                              \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (location_tracker, error_tracker, lexer_ctx, sm, sb, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use ALPHA_YYerror or ALPHA_YYUNDEF. */
#define YYERRCODE ALPHA_YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if ALPHA_YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined ALPHA_YYLTYPE_IS_TRIVIAL && ALPHA_YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, location_tracker, error_tracker, lexer_ctx, sm, sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, Alpha::LocationTracker &location_tracker, Alpha::ErrorTracker &error_tracker, Alpha::LexerCtx &lexer_ctx, Alpha::SemanticManager &sm, Alpha::SemanticBuilder &sb)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (location_tracker);
  YY_USE (error_tracker);
  YY_USE (lexer_ctx);
  YY_USE (sm);
  YY_USE (sb);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, Alpha::LocationTracker &location_tracker, Alpha::ErrorTracker &error_tracker, Alpha::LexerCtx &lexer_ctx, Alpha::SemanticManager &sm, Alpha::SemanticBuilder &sb)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, location_tracker, error_tracker, lexer_ctx, sm, sb);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, Alpha::LocationTracker &location_tracker, Alpha::ErrorTracker &error_tracker, Alpha::LexerCtx &lexer_ctx, Alpha::SemanticManager &sm, Alpha::SemanticBuilder &sb)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), location_tracker, error_tracker, lexer_ctx, sm, sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, location_tracker, error_tracker, lexer_ctx, sm, sb); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !ALPHA_YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !ALPHA_YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Given a state stack such that *YYBOTTOM is its bottom, such that
   *YYTOP is either its top or is YYTOP_EMPTY to indicate an empty
   stack, and such that *YYCAPACITY is the maximum number of elements it
   can hold without a reallocation, make sure there is enough room to
   store YYADD more elements.  If not, allocate a new stack using
   YYSTACK_ALLOC, copy the existing elements, and adjust *YYBOTTOM,
   *YYTOP, and *YYCAPACITY to reflect the new capacity and memory
   location.  If *YYBOTTOM != YYBOTTOM_NO_FREE, then free the old stack
   using YYSTACK_FREE.  Return 0 if successful or if no reallocation is
   required.  Return YYENOMEM if memory is exhausted.  */
static int
yy_lac_stack_realloc (YYPTRDIFF_T *yycapacity, YYPTRDIFF_T yyadd,
#if ALPHA_YYDEBUG
                      char const *yydebug_prefix,
                      char const *yydebug_suffix,
#endif
                      yy_state_t **yybottom,
                      yy_state_t *yybottom_no_free,
                      yy_state_t **yytop, yy_state_t *yytop_empty)
{
  YYPTRDIFF_T yysize_old =
    *yytop == yytop_empty ? 0 : *yytop - *yybottom + 1;
  YYPTRDIFF_T yysize_new = yysize_old + yyadd;
  if (*yycapacity < yysize_new)
    {
      YYPTRDIFF_T yyalloc = 2 * yysize_new;
      yy_state_t *yybottom_new;
      /* Use YYMAXDEPTH for maximum stack size given that the stack
         should never need to grow larger than the main state stack
         needs to grow without LAC.  */
      if (YYMAXDEPTH < yysize_new)
        {
          YYDPRINTF ((stderr, "%smax size exceeded%s", yydebug_prefix,
                      yydebug_suffix));
          return YYENOMEM;
        }
      if (YYMAXDEPTH < yyalloc)
        yyalloc = YYMAXDEPTH;
      yybottom_new =
        YY_CAST (yy_state_t *,
                 YYSTACK_ALLOC (YY_CAST (YYSIZE_T,
                                         yyalloc * YYSIZEOF (*yybottom_new))));
      if (!yybottom_new)
        {
          YYDPRINTF ((stderr, "%srealloc failed%s", yydebug_prefix,
                      yydebug_suffix));
          return YYENOMEM;
        }
      if (*yytop != yytop_empty)
        {
          YYCOPY (yybottom_new, *yybottom, yysize_old);
          *yytop = yybottom_new + (yysize_old - 1);
        }
      if (*yybottom != yybottom_no_free)
        YYSTACK_FREE (*yybottom);
      *yybottom = yybottom_new;
      *yycapacity = yyalloc;
    }
  return 0;
}

/* Establish the initial context for the current lookahead if no initial
   context is currently established.

   We define a context as a snapshot of the parser stacks.  We define
   the initial context for a lookahead as the context in which the
   parser initially examines that lookahead in order to select a
   syntactic action.  Thus, if the lookahead eventually proves
   syntactically unacceptable (possibly in a later context reached via a
   series of reductions), the initial context can be used to determine
   the exact set of tokens that would be syntactically acceptable in the
   lookahead's place.  Moreover, it is the context after which any
   further semantic actions would be erroneous because they would be
   determined by a syntactically unacceptable token.

   YY_LAC_ESTABLISH should be invoked when a reduction is about to be
   performed in an inconsistent state (which, for the purposes of LAC,
   includes consistent states that don't know they're consistent because
   their default reductions have been disabled).  Iff there is a
   lookahead token, it should also be invoked before reporting a syntax
   error.  This latter case is for the sake of the debugging output.

   For parse.lac=full, the implementation of YY_LAC_ESTABLISH is as
   follows.  If no initial context is currently established for the
   current lookahead, then check if that lookahead can eventually be
   shifted if syntactic actions continue from the current context.
   Report a syntax error if it cannot.  */
#define YY_LAC_ESTABLISH                                                \
do {                                                                    \
  if (!yy_lac_established)                                              \
    {                                                                   \
      YYDPRINTF ((stderr,                                               \
                  "LAC: initial context established for %s\n",          \
                  yysymbol_name (yytoken)));                            \
      yy_lac_established = 1;                                           \
      switch (yy_lac (yyesa, &yyes, &yyes_capacity, yyssp, yytoken))    \
        {                                                               \
        case YYENOMEM:                                                  \
          YYNOMEM;                                                      \
        case 1:                                                         \
          goto yyerrlab;                                                \
        }                                                               \
    }                                                                   \
} while (0)

/* Discard any previous initial lookahead context because of Event,
   which may be a lookahead change or an invalidation of the currently
   established initial context for the current lookahead.

   The most common example of a lookahead change is a shift.  An example
   of both cases is syntax error recovery.  That is, a syntax error
   occurs when the lookahead is syntactically erroneous for the
   currently established initial context, so error recovery manipulates
   the parser stacks to try to find a new initial context in which the
   current lookahead is syntactically acceptable.  If it fails to find
   such a context, it discards the lookahead.  */
#if ALPHA_YYDEBUG
# define YY_LAC_DISCARD(Event)                                           \
do {                                                                     \
  if (yy_lac_established)                                                \
    {                                                                    \
      YYDPRINTF ((stderr, "LAC: initial context discarded due to "       \
                  Event "\n"));                                          \
      yy_lac_established = 0;                                            \
    }                                                                    \
} while (0)
#else
# define YY_LAC_DISCARD(Event) yy_lac_established = 0
#endif

/* Given the stack whose top is *YYSSP, return 0 iff YYTOKEN can
   eventually (after perhaps some reductions) be shifted, return 1 if
   not, or return YYENOMEM if memory is exhausted.  As preconditions and
   postconditions: *YYES_CAPACITY is the allocated size of the array to
   which *YYES points, and either *YYES = YYESA or *YYES points to an
   array allocated with YYSTACK_ALLOC.  yy_lac may overwrite the
   contents of either array, alter *YYES and *YYES_CAPACITY, and free
   any old *YYES other than YYESA.  */
static int
yy_lac (yy_state_t *yyesa, yy_state_t **yyes,
        YYPTRDIFF_T *yyes_capacity, yy_state_t *yyssp, yysymbol_kind_t yytoken)
{
  yy_state_t *yyes_prev = yyssp;
  yy_state_t *yyesp = yyes_prev;
  /* Reduce until we encounter a shift and thereby accept the token.  */
  YYDPRINTF ((stderr, "LAC: checking lookahead %s:", yysymbol_name (yytoken)));
  if (yytoken == YYSYMBOL_YYUNDEF)
    {
      YYDPRINTF ((stderr, " Always Err\n"));
      return 1;
    }
  while (1)
    {
      int yyrule = yypact[+*yyesp];
      if (yypact_value_is_default (yyrule)
          || (yyrule += yytoken) < 0 || YYLAST < yyrule
          || yycheck[yyrule] != yytoken)
        {
          /* Use the default action.  */
          yyrule = yydefact[+*yyesp];
          if (yyrule == 0)
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
        }
      else
        {
          /* Use the action from yytable.  */
          yyrule = yytable[yyrule];
          if (yytable_value_is_error (yyrule))
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
          if (0 < yyrule)
            {
              YYDPRINTF ((stderr, " S%d\n", yyrule));
              return 0;
            }
          yyrule = -yyrule;
        }
      /* By now we know we have to simulate a reduce.  */
      YYDPRINTF ((stderr, " R%d", yyrule - 1));
      {
        /* Pop the corresponding number of values from the stack.  */
        YYPTRDIFF_T yylen = yyr2[yyrule];
        /* First pop from the LAC stack as many tokens as possible.  */
        if (yyesp != yyes_prev)
          {
            YYPTRDIFF_T yysize = yyesp - *yyes + 1;
            if (yylen < yysize)
              {
                yyesp -= yylen;
                yylen = 0;
              }
            else
              {
                yyesp = yyes_prev;
                yylen -= yysize;
              }
          }
        /* Only afterwards look at the main stack.  */
        if (yylen)
          yyesp = yyes_prev -= yylen;
      }
      /* Push the resulting state of the reduction.  */
      {
        yy_state_fast_t yystate;
        {
          const int yylhs = yyr1[yyrule] - YYNTOKENS;
          const int yyi = yypgoto[yylhs] + *yyesp;
          yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyesp
                     ? yytable[yyi]
                     : yydefgoto[yylhs]);
        }
        if (yyesp == yyes_prev)
          {
            yyesp = *yyes;
            YY_IGNORE_USELESS_CAST_BEGIN
            *yyesp = YY_CAST (yy_state_t, yystate);
            YY_IGNORE_USELESS_CAST_END
          }
        else
          {
            if (yy_lac_stack_realloc (yyes_capacity, 1,
#if ALPHA_YYDEBUG
                                      " (", ")",
#endif
                                      yyes, yyesa, &yyesp, yyes_prev))
              {
                YYDPRINTF ((stderr, "\n"));
                return YYENOMEM;
              }
            YY_IGNORE_USELESS_CAST_BEGIN
            *++yyesp = YY_CAST (yy_state_t, yystate);
            YY_IGNORE_USELESS_CAST_END
          }
        YYDPRINTF ((stderr, " G%d", yystate));
      }
    }
}

/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yy_state_t *yyesa;
  yy_state_t **yyes;
  YYPTRDIFF_T *yyes_capacity;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;

  int yyx;
  for (yyx = 0; yyx < YYNTOKENS; ++yyx)
    {
      yysymbol_kind_t yysym = YY_CAST (yysymbol_kind_t, yyx);
      if (yysym != YYSYMBOL_YYerror && yysym != YYSYMBOL_YYUNDEF)
        switch (yy_lac (yyctx->yyesa, yyctx->yyes, yyctx->yyes_capacity, yyctx->yyssp, yysym))
          {
          case YYENOMEM:
            return YYENOMEM;
          case 1:
            continue;
          default:
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
       In the first two cases, it might appear that the current syntax
       error should have been detected in the previous state when yy_lac
       was invoked.  However, at that time, there might have been a
       different syntax error that discarded a different initial context
       during error recovery, leaving behind the current lookahead.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      YYDPRINTF ((stderr, "Constructing syntax error message\n"));
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else if (yyn == 0)
        YYDPRINTF ((stderr, "No expected tokens.\n"));
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.  In order to see if a particular token T is a
   valid looakhead, invoke yy_lac (YYESA, YYES, YYES_CAPACITY, YYSSP, T).

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store or if
   yy_lac returned YYENOMEM.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, Alpha::LocationTracker &location_tracker, Alpha::ErrorTracker &error_tracker, Alpha::LexerCtx &lexer_ctx, Alpha::SemanticManager &sm, Alpha::SemanticBuilder &sb)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (location_tracker);
  YY_USE (error_tracker);
  YY_USE (lexer_ctx);
  YY_USE (sm);
  YY_USE (sb);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined ALPHA_YYLTYPE_IS_TRIVIAL && ALPHA_YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (Alpha::LocationTracker &location_tracker, Alpha::ErrorTracker &error_tracker, Alpha::LexerCtx &lexer_ctx, Alpha::SemanticManager &sm, Alpha::SemanticBuilder &sb)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

    yy_state_t yyesa[20];
    yy_state_t *yyes = yyesa;
    YYPTRDIFF_T yyes_capacity = 20 < YYMAXDEPTH ? 20 : YYMAXDEPTH;

  /* Whether LAC context is established.  A Boolean.  */
  int yy_lac_established = 0;
  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = ALPHA_YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == ALPHA_YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (location_tracker, error_tracker, lexer_ctx);
    }

  if (yychar <= ALPHA_YYEOF)
    {
      yychar = ALPHA_YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == ALPHA_YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = ALPHA_YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    {
      YY_LAC_ESTABLISH;
      goto yydefault;
    }
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      YY_LAC_ESTABLISH;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = ALPHA_YYEMPTY;
  YY_LAC_DISCARD ("shift");
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  {
    int yychar_backup = yychar;
    switch (yyn)
      {
  case 4: /* multiStmt: stmt  */
#line 192 "alpha_parser_spec.y"
  { sm.multiStmt__stmt(); }
#line 2163 "alpha_parser_spec.tab.c"
    break;

  case 5: /* $@1: %empty  */
#line 194 "alpha_parser_spec.y"
  { sm.multiStmt__stmt(); }
#line 2169 "alpha_parser_spec.tab.c"
    break;

  case 7: /* stmt: expr ";"  */
#line 199 "alpha_parser_spec.y"
                 { sm.backpatch_bool_expr((yyvsp[-1].expr_ptr), (yylsp[-1])); }
#line 2175 "alpha_parser_spec.tab.c"
    break;

  case 16: /* stmt: error ";"  */
#line 208 "alpha_parser_spec.y"
                      { yyerrok; }
#line 2181 "alpha_parser_spec.tab.c"
    break;

  case 17: /* stmt: error ")"  */
#line 209 "alpha_parser_spec.y"
                      { yyerrok; }
#line 2187 "alpha_parser_spec.tab.c"
    break;

  case 18: /* stmt: error "]"  */
#line 210 "alpha_parser_spec.y"
                      { yyerrok; }
#line 2193 "alpha_parser_spec.tab.c"
    break;

  case 19: /* stmt: error "}"  */
#line 211 "alpha_parser_spec.y"
                      { yyerrok; }
#line 2199 "alpha_parser_spec.tab.c"
    break;

  case 20: /* loopCtrlStmt: "keyword `break`"  */
#line 215 "alpha_parser_spec.y"
            { sm.loopCtrlStmt__break((yylsp[0])); }
#line 2205 "alpha_parser_spec.tab.c"
    break;

  case 21: /* loopCtrlStmt: "keyword `continue`"  */
#line 216 "alpha_parser_spec.y"
           { sm.loopCtrlStmt__continue((yylsp[0])); }
#line 2211 "alpha_parser_spec.tab.c"
    break;

  case 23: /* expr: expr "+" expr  */
#line 221 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_arithmetic(AOP::ADD, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr), (yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2217 "alpha_parser_spec.tab.c"
    break;

  case 24: /* expr: expr "-" expr  */
#line 222 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_arithmetic(AOP::SUB, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr), (yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2223 "alpha_parser_spec.tab.c"
    break;

  case 25: /* expr: expr "*" expr  */
#line 223 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_arithmetic(AOP::MUL, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr), (yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2229 "alpha_parser_spec.tab.c"
    break;

  case 26: /* expr: expr "/" expr  */
#line 224 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_arithmetic(AOP::DIV, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr), (yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2235 "alpha_parser_spec.tab.c"
    break;

  case 27: /* expr: expr "%" expr  */
#line 225 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_arithmetic(AOP::MOD, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr), (yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2241 "alpha_parser_spec.tab.c"
    break;

  case 28: /* expr: expr "<" expr  */
#line 226 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_relational(AOP::IF_GREATER, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr),(yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2247 "alpha_parser_spec.tab.c"
    break;

  case 29: /* expr: expr ">=" expr  */
#line 227 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_relational(AOP::IF_GREATEREQ, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr),(yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2253 "alpha_parser_spec.tab.c"
    break;

  case 30: /* expr: expr ">" expr  */
#line 228 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_relational(AOP::IF_LESS, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr),(yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2259 "alpha_parser_spec.tab.c"
    break;

  case 31: /* expr: expr "<=" expr  */
#line 229 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_relational(AOP::IF_LESSEQ, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr),(yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2265 "alpha_parser_spec.tab.c"
    break;

  case 32: /* expr: expr "==" expr  */
#line 230 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_relational(AOP::IF_EQ, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr),(yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2271 "alpha_parser_spec.tab.c"
    break;

  case 33: /* expr: expr "!=" expr  */
#line 231 "alpha_parser_spec.y"
                               { (yyval.expr_ptr) = sb.make_relational(AOP::IF_NOTEQ, (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr),(yyloc), (yylsp[-2]), (yylsp[0])); }
#line 2277 "alpha_parser_spec.tab.c"
    break;

  case 34: /* expr: expr "keyword `and`" andHook expr  */
#line 235 "alpha_parser_spec.y"
  { 
    (yyvsp[0].expr_ptr) = sb.convert_to_boolean((yyvsp[0].expr_ptr), (yylsp[0]));   
    (yyval.expr_ptr) = sb.make_logical_and((yyvsp[-3].expr_ptr), (yyvsp[0].expr_ptr), (yyloc), (yylsp[-3]), (yylsp[0]));
  }
#line 2286 "alpha_parser_spec.tab.c"
    break;

  case 35: /* $@2: %empty  */
#line 240 "alpha_parser_spec.y"
  {
    (yyvsp[0].expr_ptr) = sb.convert_to_boolean((yyvsp[0].expr_ptr), (yylsp[0])); 
  }
#line 2294 "alpha_parser_spec.tab.c"
    break;

  case 36: /* expr: expr $@2 "keyword `or`" orHook expr  */
#line 245 "alpha_parser_spec.y"
  {
    (yyvsp[0].expr_ptr) = sb.convert_to_boolean((yyvsp[0].expr_ptr), (yylsp[0]));   
    (yyval.expr_ptr) = sb.make_logical_or((yyvsp[-4].expr_ptr), (yyvsp[0].expr_ptr), (yyloc), (yylsp[-4]), (yylsp[0])); 
  }
#line 2303 "alpha_parser_spec.tab.c"
    break;

  case 37: /* expr: term  */
#line 249 "alpha_parser_spec.y"
       { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2309 "alpha_parser_spec.tab.c"
    break;

  case 38: /* orHook: %empty  */
#line 253 "alpha_parser_spec.y"
  { sm.orHook(); }
#line 2315 "alpha_parser_spec.tab.c"
    break;

  case 39: /* andHook: %empty  */
#line 257 "alpha_parser_spec.y"
  { sm.andHook(); }
#line 2321 "alpha_parser_spec.tab.c"
    break;

  case 40: /* term: "(" expr ")"  */
#line 261 "alpha_parser_spec.y"
                              { (yyval.expr_ptr) = (yyvsp[-1].expr_ptr); }
#line 2327 "alpha_parser_spec.tab.c"
    break;

  case 41: /* term: "-" expr  */
#line 262 "alpha_parser_spec.y"
                              { (yyval.expr_ptr) = sb.make_uminus((yyvsp[0].expr_ptr), (yyloc), (yylsp[0])); }
#line 2333 "alpha_parser_spec.tab.c"
    break;

  case 42: /* term: "keyword `not`" expr  */
#line 263 "alpha_parser_spec.y"
                              { (yyval.expr_ptr) = sb.make_logical_not((yyvsp[0].expr_ptr), (yyloc)); }
#line 2339 "alpha_parser_spec.tab.c"
    break;

  case 43: /* term: "increment operator `++`" lvalue  */
#line 264 "alpha_parser_spec.y"
             { sm.term__inc_lvalue((yyval.expr_ptr), (yyvsp[0].expr_ptr), (yyloc)); }
#line 2345 "alpha_parser_spec.tab.c"
    break;

  case 44: /* term: lvalue "increment operator `++`"  */
#line 265 "alpha_parser_spec.y"
             { sm.term__lvalue_inc((yyval.expr_ptr), (yyvsp[-1].expr_ptr), (yyloc)); }
#line 2351 "alpha_parser_spec.tab.c"
    break;

  case 45: /* term: "decrement operator `--`" lvalue  */
#line 266 "alpha_parser_spec.y"
             { sm.term__dec_lvalue((yyval.expr_ptr), (yyvsp[0].expr_ptr), (yyloc)); }
#line 2357 "alpha_parser_spec.tab.c"
    break;

  case 46: /* term: lvalue "decrement operator `--`"  */
#line 267 "alpha_parser_spec.y"
             { sm.term__lvalue_dec((yyval.expr_ptr), (yyvsp[-1].expr_ptr), (yyloc)); }
#line 2363 "alpha_parser_spec.tab.c"
    break;

  case 47: /* term: primary  */
#line 268 "alpha_parser_spec.y"
          { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2369 "alpha_parser_spec.tab.c"
    break;

  case 48: /* assignExpr: lvalue "assignment operator =" expr  */
#line 273 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.resolve_assign_expr((yyvsp[-2].expr_ptr),(yyvsp[0].expr_ptr), (yylsp[-1])); }
#line 2375 "alpha_parser_spec.tab.c"
    break;

  case 49: /* primary: lvalue  */
#line 277 "alpha_parser_spec.y"
         { (yyval.expr_ptr) = sb.resolve_lvalue_to_primary((yyvsp[0].expr_ptr)); }
#line 2381 "alpha_parser_spec.tab.c"
    break;

  case 52: /* primary: "(" funcDef ")"  */
#line 281 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_program_function((yyvsp[-1].const_function_symbol_ptr)); }
#line 2387 "alpha_parser_spec.tab.c"
    break;

  case 53: /* primary: const  */
#line 282 "alpha_parser_spec.y"
         { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2393 "alpha_parser_spec.tab.c"
    break;

  case 54: /* lvalue: "`identifier`"  */
#line 287 "alpha_parser_spec.y"
            { sm.lvalue__id((yyval.expr_ptr), (yyvsp[0].cstring), (yylsp[0]));  std::cout << "4.PARSER: ID ==  " << (yyvsp[0].cstring) << std::endl; }
#line 2399 "alpha_parser_spec.tab.c"
    break;

  case 55: /* lvalue: "keyword `local`" "`identifier`"  */
#line 288 "alpha_parser_spec.y"
            { sm.lvalue__local_id((yyval.expr_ptr), (yyvsp[0].cstring), (yylsp[0])); std::cout << "5.PARSER: ID ==  " << (yyvsp[0].cstring) << std::endl;}
#line 2405 "alpha_parser_spec.tab.c"
    break;

  case 56: /* lvalue: "global operator ::" "`identifier`"  */
#line 289 "alpha_parser_spec.y"
            { sm.lvalue__global_id((yyval.expr_ptr), (yyvsp[0].cstring), (yylsp[0])); std::cout << "6.PARSER: ID ==  " << (yyvsp[0].cstring) << std::endl;}
#line 2411 "alpha_parser_spec.tab.c"
    break;

  case 57: /* lvalue: member  */
#line 290 "alpha_parser_spec.y"
         { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2417 "alpha_parser_spec.tab.c"
    break;

  case 58: /* tableItem: lvalue "." "`identifier`"  */
#line 295 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_table_item((yyvsp[-2].expr_ptr), (yyvsp[0].cstring), (yyloc) ,(yylsp[0])); }
#line 2423 "alpha_parser_spec.tab.c"
    break;

  case 59: /* tableItem: lvalue "[" expr "]"  */
#line 297 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_table_item((yyvsp[-3].expr_ptr), (yyvsp[-1].expr_ptr), (yyloc)); }
#line 2429 "alpha_parser_spec.tab.c"
    break;

  case 60: /* member: tableItem  */
#line 301 "alpha_parser_spec.y"
            { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2435 "alpha_parser_spec.tab.c"
    break;

  case 63: /* methodCallId: "method-call operator .." "`identifier`"  */
#line 308 "alpha_parser_spec.y"
  { sm.methodCallId__methodcall_id((yyvsp[0].cstring), (yylsp[0]), (yyloc));}
#line 2441 "alpha_parser_spec.tab.c"
    break;

  case 64: /* call: call "(" elist ")"  */
#line 314 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_call((yyvsp[-3].expr_ptr), (yyvsp[-1].expr_list_ptr), (yyloc)); }
#line 2447 "alpha_parser_spec.tab.c"
    break;

  case 65: /* call: lvalue "(" elist ")"  */
#line 316 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_normal_call((yyvsp[-3].expr_ptr), (yyvsp[-1].expr_list_ptr), (yyloc)); }
#line 2453 "alpha_parser_spec.tab.c"
    break;

  case 66: /* call: lvalue methodCallId "(" elist ")"  */
#line 318 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_method_call((yyvsp[-4].expr_ptr), (yyvsp[-1].expr_list_ptr), (yyloc)); }
#line 2459 "alpha_parser_spec.tab.c"
    break;

  case 67: /* call: "(" funcDef ")" "(" elist ")"  */
#line 320 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_iife_call((yyvsp[-4].const_function_symbol_ptr), (yyvsp[-1].expr_list_ptr), (yyloc)); }
#line 2465 "alpha_parser_spec.tab.c"
    break;

  case 68: /* exprList: expr  */
#line 325 "alpha_parser_spec.y"
  { (yyval.expr_list_ptr) = sb.make_expr_list_with((yyvsp[0].expr_ptr), (yylsp[0])); }
#line 2471 "alpha_parser_spec.tab.c"
    break;

  case 69: /* exprList: expr "," exprList  */
#line 327 "alpha_parser_spec.y"
  { (yyval.expr_list_ptr) = sb.extend_expr_list_with((yyvsp[0].expr_list_ptr), (yyvsp[-2].expr_ptr), (yylsp[-2])); }
#line 2477 "alpha_parser_spec.tab.c"
    break;

  case 70: /* elist: %empty  */
#line 331 "alpha_parser_spec.y"
                { (yyval.expr_list_ptr) = sb.make_empty_expr_list(); }
#line 2483 "alpha_parser_spec.tab.c"
    break;

  case 71: /* elist: exprList  */
#line 332 "alpha_parser_spec.y"
                { (yyval.expr_list_ptr) = (yyvsp[0].expr_list_ptr); }
#line 2489 "alpha_parser_spec.tab.c"
    break;

  case 72: /* tableList: "[" elist "]"  */
#line 337 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_table_list((yyvsp[-1].expr_list_ptr), (yyloc)); }
#line 2495 "alpha_parser_spec.tab.c"
    break;

  case 73: /* tableDict: "[" indexed "]"  */
#line 342 "alpha_parser_spec.y"
  { (yyval.expr_ptr) = sb.make_table_dict((yyvsp[-1].dict_list_ptr), (yyloc)); }
#line 2501 "alpha_parser_spec.tab.c"
    break;

  case 74: /* objectDef: tableList  */
#line 346 "alpha_parser_spec.y"
            { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2507 "alpha_parser_spec.tab.c"
    break;

  case 75: /* objectDef: tableDict  */
#line 347 "alpha_parser_spec.y"
            { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2513 "alpha_parser_spec.tab.c"
    break;

  case 76: /* indexed: indexedElemList  */
#line 351 "alpha_parser_spec.y"
                  { (yyval.dict_list_ptr) = (yyvsp[0].dict_list_ptr); }
#line 2519 "alpha_parser_spec.tab.c"
    break;

  case 77: /* indexedElemList: indexedElem  */
#line 356 "alpha_parser_spec.y"
  { (yyval.dict_list_ptr) = sb.make_dict_list_with((yyvsp[0].expr_pair_ptr)); }
#line 2525 "alpha_parser_spec.tab.c"
    break;

  case 78: /* indexedElemList: indexedElem "," indexedElemList  */
#line 358 "alpha_parser_spec.y"
  { (yyval.dict_list_ptr) = sb.extend_dict_list_with((yyvsp[0].dict_list_ptr), (yyvsp[-2].expr_pair_ptr)); }
#line 2531 "alpha_parser_spec.tab.c"
    break;

  case 79: /* indexedElem: "{" expr ":" expr "}"  */
#line 363 "alpha_parser_spec.y"
  { (yyval.expr_pair_ptr) = sb.make_expr_pair((yyvsp[-3].expr_ptr), (yyvsp[-1].expr_ptr)); }
#line 2537 "alpha_parser_spec.tab.c"
    break;

  case 80: /* blockBegin: "{"  */
#line 368 "alpha_parser_spec.y"
  { 
    sm.blockBegin__lbrace();
    (yyval.location) = (yylsp[0]);
  }
#line 2546 "alpha_parser_spec.tab.c"
    break;

  case 81: /* blockEnd: "}"  */
#line 376 "alpha_parser_spec.y"
  { 
    sm.blockEnd__rbrace();
    (yyval.location) = (yylsp[0]);
  }
#line 2555 "alpha_parser_spec.tab.c"
    break;

  case 82: /* block: blockBegin multiStmt blockEnd  */
#line 384 "alpha_parser_spec.y"
  { (yyval.block_location) = sb.make_block_location((yyvsp[-2].location), (yyvsp[0].location)); }
#line 2561 "alpha_parser_spec.tab.c"
    break;

  case 83: /* block: blockBegin blockEnd  */
#line 386 "alpha_parser_spec.y"
  { (yyval.block_location) = sb.make_block_location((yyvsp[-1].location), (yyvsp[0].location)); }
#line 2567 "alpha_parser_spec.tab.c"
    break;

  case 84: /* funcPrefix: "keyword `function`"  */
#line 391 "alpha_parser_spec.y"
              { sm.funcPrefix__function((yylsp[0])); }
#line 2573 "alpha_parser_spec.tab.c"
    break;

  case 85: /* funcPrefix: "keyword `function`" "`identifier`"  */
#line 392 "alpha_parser_spec.y"
              { sm.funcPrefix__function_id((yyvsp[0].cstring), (yylsp[0])); }
#line 2579 "alpha_parser_spec.tab.c"
    break;

  case 86: /* funcArgs: "`identifier`"  */
#line 396 "alpha_parser_spec.y"
     { sm.funcArgs__id((yyvsp[0].cstring), (yylsp[0])); }
#line 2585 "alpha_parser_spec.tab.c"
    break;

  case 87: /* $@3: %empty  */
#line 397 "alpha_parser_spec.y"
     { sm.funcArgs__id((yyvsp[0].cstring), (yylsp[0])); }
#line 2591 "alpha_parser_spec.tab.c"
    break;

  case 91: /* funcSignature: funcPrefix funcArgList  */
#line 407 "alpha_parser_spec.y"
  { sm.funcSignature__funcPrefix_funcArgList((yyval.const_function_symbol_ptr)); }
#line 2597 "alpha_parser_spec.tab.c"
    break;

  case 92: /* funcDef: funcSignature block  */
#line 412 "alpha_parser_spec.y"
  { 
    // TODO to much inderection remove funcSignature rule and MERGE... 
    sm.funcDef__funcSignature_block((yyvsp[0].block_location)); 
    (yyval.const_function_symbol_ptr) = (yyvsp[-1].const_function_symbol_ptr);
  }
#line 2607 "alpha_parser_spec.tab.c"
    break;

  case 93: /* const: "keyword `nil`"  */
#line 420 "alpha_parser_spec.y"
         { (yyval.expr_ptr) = sb.make_const_nil((yylsp[0])); }
#line 2613 "alpha_parser_spec.tab.c"
    break;

  case 94: /* const: "keyword `true`"  */
#line 421 "alpha_parser_spec.y"
         { (yyval.expr_ptr) = sb.make_const_true((yylsp[0])); }
#line 2619 "alpha_parser_spec.tab.c"
    break;

  case 95: /* const: "keyword `false`"  */
#line 422 "alpha_parser_spec.y"
         { (yyval.expr_ptr) = sb.make_const_false((yylsp[0])); }
#line 2625 "alpha_parser_spec.tab.c"
    break;

  case 96: /* const: "`integer-constant`"  */
#line 423 "alpha_parser_spec.y"
         { (yyval.expr_ptr) = sb.make_const_int((yyvsp[0].const_int), (yylsp[0])); }
#line 2631 "alpha_parser_spec.tab.c"
    break;

  case 97: /* const: "`real-constant`"  */
#line 424 "alpha_parser_spec.y"
         { (yyval.expr_ptr) = sb.make_const_real((yyvsp[0].const_real), (yylsp[0])); }
#line 2637 "alpha_parser_spec.tab.c"
    break;

  case 98: /* const: "`string-literal`"  */
#line 425 "alpha_parser_spec.y"
         {

//TODO. STOP LEXER FROM COMPYING THE STRING.. IT USESLESS and MAKES US NEED EXTRA Deaclocation bookeeping 

   (yyval.expr_ptr) = sb.make_const_string((yyvsp[0].cstring), (yylsp[0])); delete[] (yyvsp[0].cstring); (yyvsp[0].cstring) = nullptr; }
#line 2647 "alpha_parser_spec.tab.c"
    break;

  case 99: /* ifPrefix: "keyword `if`" "(" expr ")"  */
#line 433 "alpha_parser_spec.y"
                                 { 
  std::cout << "WELL I RUNNN"<<std::endl;
  sm.ifPrefix__if_lparen_expr_rparen((yyvsp[-1].expr_ptr), (yylsp[-1])); }
#line 2655 "alpha_parser_spec.tab.c"
    break;

  case 100: /* elsePrefix: "keyword `else`"  */
#line 438 "alpha_parser_spec.y"
       { sm.elsePrefix__else((yylsp[0])); }
#line 2661 "alpha_parser_spec.tab.c"
    break;

  case 101: /* ifStmt: ifPrefix stmt  */
#line 442 "alpha_parser_spec.y"
                           { sm.ifStmt__ifPrefix_stmt_then(); }
#line 2667 "alpha_parser_spec.tab.c"
    break;

  case 102: /* ifStmt: ifPrefix stmt elsePrefix stmt  */
#line 443 "alpha_parser_spec.y"
                                 { sm.ifStmt__ifPrefix_stmt_elsePrefix_stmt(); }
#line 2673 "alpha_parser_spec.tab.c"
    break;

  case 104: /* $@4: %empty  */
#line 452 "alpha_parser_spec.y"
  { sm.whileStmt__whileHeader(); }
#line 2679 "alpha_parser_spec.tab.c"
    break;

  case 105: /* whileStmt: whileHeader $@4 stmt  */
#line 454 "alpha_parser_spec.y"
  { sm.whileStmt__whileHeader_stmt(); }
#line 2685 "alpha_parser_spec.tab.c"
    break;

  case 107: /* $@5: %empty  */
#line 463 "alpha_parser_spec.y"
  { sm.forStmt__forHeader(); }
#line 2691 "alpha_parser_spec.tab.c"
    break;

  case 108: /* forStmt: forHeader $@5 stmt  */
#line 465 "alpha_parser_spec.y"
  { sm.forStmt__forHeader_stmt(); }
#line 2697 "alpha_parser_spec.tab.c"
    break;

  case 109: /* funcCtrlStmt: "keyword `return`"  */
#line 469 "alpha_parser_spec.y"
         { sm.funcCtrlStmt__return((yylsp[0])); }
#line 2703 "alpha_parser_spec.tab.c"
    break;


#line 2707 "alpha_parser_spec.tab.c"

        default: break;
      }
    if (yychar_backup != yychar)
      YY_LAC_DISCARD ("yychar change");
  }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == ALPHA_YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yyesa, &yyes, &yyes_capacity, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        if (yychar != ALPHA_YYEMPTY)
          YY_LAC_ESTABLISH;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (location_tracker, error_tracker, lexer_ctx, sm, sb, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= ALPHA_YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == ALPHA_YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, location_tracker, error_tracker, lexer_ctx, sm, sb);
          yychar = ALPHA_YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, location_tracker, error_tracker, lexer_ctx, sm, sb);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  /* If the stack popping above didn't lose the initial context for the
     current lookahead token, the shift below will for sure.  */
  YY_LAC_DISCARD ("error recovery");

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (location_tracker, error_tracker, lexer_ctx, sm, sb, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != ALPHA_YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, location_tracker, error_tracker, lexer_ctx, sm, sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, location_tracker, error_tracker, lexer_ctx, sm, sb);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yyes != yyesa)
    YYSTACK_FREE (yyes);
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 478 "alpha_parser_spec.y"

