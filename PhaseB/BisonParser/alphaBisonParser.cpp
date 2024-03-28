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

/* Substitute the type names.  */
#define YYSTYPE         ALPHA_YYSTYPE
/* Substitute the variable and function names.  */
#define yyparse         alpha_yyparse
#define yylex           alpha_yylex
#define yyerror         alpha_yyerror
#define yydebug         alpha_yydebug
#define yynerrs         alpha_yynerrs
#define yylval          alpha_yylval
#define yychar          alpha_yychar

/* First part of user prologue.  */
#line 1 "alphaBisonParser.y"

        #define INSIDE_BISON_FILE
        #include <string>
        #include <iostream>
        #include "../FlexScanner/alphaFlexScanner.hpp"
        #include "../alphaDefs.hpp"

        int yyerror(std::string message);

#line 89 "alphaBisonParser.cpp"

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

#include "alphaBisonParser.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_STRING_LITERAL = 3,             /* STRING_LITERAL  */
  YYSYMBOL_ID = 4,                         /* ID  */
  YYSYMBOL_INT_CONST = 5,                  /* INT_CONST  */
  YYSYMBOL_REAL_CONST = 6,                 /* REAL_CONST  */
  YYSYMBOL_IF = 7,                         /* IF  */
  YYSYMBOL_ELSE = 8,                       /* ELSE  */
  YYSYMBOL_WHILE = 9,                      /* WHILE  */
  YYSYMBOL_FOR = 10,                       /* FOR  */
  YYSYMBOL_FUNCTION = 11,                  /* FUNCTION  */
  YYSYMBOL_RETURN = 12,                    /* RETURN  */
  YYSYMBOL_BREAK = 13,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 14,                  /* CONTINUE  */
  YYSYMBOL_AND = 15,                       /* AND  */
  YYSYMBOL_NOT = 16,                       /* NOT  */
  YYSYMBOL_OR = 17,                        /* OR  */
  YYSYMBOL_LOCAL = 18,                     /* LOCAL  */
  YYSYMBOL_TRUE = 19,                      /* TRUE  */
  YYSYMBOL_FALSE = 20,                     /* FALSE  */
  YYSYMBOL_NIL = 21,                       /* NIL  */
  YYSYMBOL_ASSIGN = 22,                    /* ASSIGN  */
  YYSYMBOL_PLUS = 23,                      /* PLUS  */
  YYSYMBOL_MINUS = 24,                     /* MINUS  */
  YYSYMBOL_MUL = 25,                       /* MUL  */
  YYSYMBOL_DIV = 26,                       /* DIV  */
  YYSYMBOL_MOD = 27,                       /* MOD  */
  YYSYMBOL_EQUAL = 28,                     /* EQUAL  */
  YYSYMBOL_NOT_EQUAL = 29,                 /* NOT_EQUAL  */
  YYSYMBOL_PLUS_PLUS = 30,                 /* PLUS_PLUS  */
  YYSYMBOL_MINUS_MINUS = 31,               /* MINUS_MINUS  */
  YYSYMBOL_GREATER_THAN = 32,              /* GREATER_THAN  */
  YYSYMBOL_LESS_THAN = 33,                 /* LESS_THAN  */
  YYSYMBOL_GREATER_THAN_OR_EQUAL = 34,     /* GREATER_THAN_OR_EQUAL  */
  YYSYMBOL_LESS_THAN_OR_EQUAL = 35,        /* LESS_THAN_OR_EQUAL  */
  YYSYMBOL_LEFT_BRACE = 36,                /* LEFT_BRACE  */
  YYSYMBOL_RIGHT_BRACE = 37,               /* RIGHT_BRACE  */
  YYSYMBOL_LEFT_BRACKET = 38,              /* LEFT_BRACKET  */
  YYSYMBOL_RIGHT_BRACKET = 39,             /* RIGHT_BRACKET  */
  YYSYMBOL_LEFT_PARENTHESIS = 40,          /* LEFT_PARENTHESIS  */
  YYSYMBOL_RIGHT_PARENTHESIS = 41,         /* RIGHT_PARENTHESIS  */
  YYSYMBOL_SEMI_COLON = 42,                /* SEMI_COLON  */
  YYSYMBOL_COMMA = 43,                     /* COMMA  */
  YYSYMBOL_COLON = 44,                     /* COLON  */
  YYSYMBOL_COLON_BLOCK = 45,               /* COLON_BLOCK  */
  YYSYMBOL_DOT = 46,                       /* DOT  */
  YYSYMBOL_DDOT = 47,                      /* DDOT  */
  YYSYMBOL_YYACCEPT = 48,                  /* $accept  */
  YYSYMBOL_program = 49,                   /* program  */
  YYSYMBOL_stmt = 50,                      /* stmt  */
  YYSYMBOL_multi_stmt = 51,                /* multi_stmt  */
  YYSYMBOL_expr = 52,                      /* expr  */
  YYSYMBOL_term = 53,                      /* term  */
  YYSYMBOL_assignexpr = 54,                /* assignexpr  */
  YYSYMBOL_primary = 55,                   /* primary  */
  YYSYMBOL_lvalue = 56,                    /* lvalue  */
  YYSYMBOL_member = 57,                    /* member  */
  YYSYMBOL_call = 58,                      /* call  */
  YYSYMBOL_callsuffix = 59,                /* callsuffix  */
  YYSYMBOL_normcall = 60,                  /* normcall  */
  YYSYMBOL_methodcall = 61,                /* methodcall  */
  YYSYMBOL_expr_list = 62,                 /* expr_list  */
  YYSYMBOL_elist = 63,                     /* elist  */
  YYSYMBOL_objectdef = 64,                 /* objectdef  */
  YYSYMBOL_indexed = 65,                   /* indexed  */
  YYSYMBOL_indexedelem = 66,               /* indexedelem  */
  YYSYMBOL_block = 67,                     /* block  */
  YYSYMBOL_funcdef = 68,                   /* funcdef  */
  YYSYMBOL_const = 69,                     /* const  */
  YYSYMBOL_idlist = 70,                    /* idlist  */
  YYSYMBOL_ifstmt = 71,                    /* ifstmt  */
  YYSYMBOL_whilestmt = 72,                 /* whilestmt  */
  YYSYMBOL_forstmt = 73,                   /* forstmt  */
  YYSYMBOL_returnstmt = 74,                /* returnstmt  */
  YYSYMBOL_indexedelem_list = 75           /* indexedelem_list  */
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

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined ALPHA_YYSTYPE_IS_TRIVIAL && ALPHA_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  70
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   562

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  48
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  28
/* YYNRULES -- Number of rules.  */
#define YYNRULES  86
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  171

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   302


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
      45,    46,    47
};

#if ALPHA_YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   100,   100,   101,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   116,   117,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   137,   138,   139,   140,   141,   142,   143,   144,   147,
     150,   151,   152,   153,   154,   157,   158,   159,   160,   163,
     164,   165,   166,   169,   170,   171,   175,   176,   179,   182,
     185,   186,   189,   190,   193,   194,   197,   200,   203,   206,
     207,   210,   211,   212,   213,   214,   215,   218,   219,   222,
     223,   226,   229,   232,   233,   236,   237
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if ALPHA_YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "STRING_LITERAL", "ID",
  "INT_CONST", "REAL_CONST", "IF", "ELSE", "WHILE", "FOR", "FUNCTION",
  "RETURN", "BREAK", "CONTINUE", "AND", "NOT", "OR", "LOCAL", "TRUE",
  "FALSE", "NIL", "ASSIGN", "PLUS", "MINUS", "MUL", "DIV", "MOD", "EQUAL",
  "NOT_EQUAL", "PLUS_PLUS", "MINUS_MINUS", "GREATER_THAN", "LESS_THAN",
  "GREATER_THAN_OR_EQUAL", "LESS_THAN_OR_EQUAL", "LEFT_BRACE",
  "RIGHT_BRACE", "LEFT_BRACKET", "RIGHT_BRACKET", "LEFT_PARENTHESIS",
  "RIGHT_PARENTHESIS", "SEMI_COLON", "COMMA", "COLON", "COLON_BLOCK",
  "DOT", "DDOT", "$accept", "program", "stmt", "multi_stmt", "expr",
  "term", "assignexpr", "primary", "lvalue", "member", "call",
  "callsuffix", "normcall", "methodcall", "expr_list", "elist",
  "objectdef", "indexed", "indexedelem", "block", "funcdef", "const",
  "idlist", "ifstmt", "whilestmt", "forstmt", "returnstmt",
  "indexedelem_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-135)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     110,  -135,  -135,  -135,  -135,   -34,   -30,   -17,    -1,   154,
      -9,    -8,   230,    32,  -135,  -135,  -135,   230,     7,     7,
     110,   184,   207,  -135,    34,    35,   110,  -135,   297,  -135,
    -135,  -135,    63,  -135,   -22,  -135,  -135,  -135,  -135,  -135,
    -135,  -135,  -135,   230,   230,   230,     2,    41,  -135,   318,
    -135,  -135,  -135,  -135,     5,    37,    21,   -22,    21,    12,
     230,   276,  -135,    11,    23,     8,  -135,   360,    28,  -135,
    -135,  -135,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,  -135,   230,  -135,  -135,   230,
     230,    66,    67,  -135,  -135,  -135,   230,   230,    83,   381,
     402,    46,    41,    47,    50,  -135,    51,  -135,   254,   230,
    -135,  -135,    59,  -135,    57,   514,   501,     5,     5,  -135,
    -135,  -135,   527,   527,    31,    31,    31,    31,   486,   423,
      58,  -135,    60,   444,    65,  -135,   110,   110,   230,    70,
      41,    62,    57,   230,  -135,  -135,   230,  -135,  -135,   230,
    -135,  -135,    96,  -135,   339,    62,  -135,  -135,   465,    71,
      84,   110,   230,  -135,  -135,  -135,  -135,  -135,    86,   110,
    -135
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,    73,    45,    71,    72,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    74,     0,     0,     0,
       0,    63,     0,    13,     0,     0,    14,     3,     0,    30,
      16,    38,    40,    48,    41,    42,    11,    12,    44,     5,
       6,     7,     8,     0,     0,    63,     0,     0,    83,     0,
       9,    10,    33,    46,    32,     0,    34,     0,    36,     0,
       0,    60,    62,     0,     0,    85,    66,     0,     0,    47,
       1,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     0,    35,    37,     0,
      63,     0,     0,    54,    56,    57,     0,    63,     0,     0,
       0,     0,     0,    77,     0,    84,     0,    68,     0,     0,
      64,    65,     0,    31,    43,    28,    29,    17,    18,    19,
      20,    21,    26,    27,    22,    24,    23,    25,    39,     0,
       0,    49,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,     0,    61,    86,    63,    50,    58,    63,
      52,    53,    79,    81,     0,     0,    78,    70,     0,     0,
       0,     0,    63,    69,    67,    55,    59,    80,     0,     0,
      82
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -135,  -135,  -132,    -7,     0,  -135,  -135,  -135,    -4,  -135,
       9,  -135,  -135,  -135,    -2,   -44,  -135,  -135,  -135,  -134,
     -14,  -135,  -100,  -135,  -135,  -135,  -135,    20
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    25,    26,    27,    61,    29,    30,    31,    32,    33,
      34,    93,    94,    95,    62,    63,    35,    64,    65,    36,
      37,    38,   104,    39,    40,    41,    42,    66
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      28,   101,   139,    46,   152,   153,    43,   157,    68,    49,
      44,     2,    52,    59,    56,    58,    96,    54,    97,    71,
      28,   163,    67,    45,    98,    13,    28,    57,    57,   167,
      76,    77,    78,    50,    51,    70,    53,   170,    69,    47,
     156,   106,   102,    99,   100,   103,   130,    55,     8,   107,
     110,   112,    24,   134,    74,    75,    76,    77,    78,    89,
     108,    90,   111,    -1,    -1,    -1,    -1,    91,    92,   114,
     131,   132,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,    86,   128,   135,   138,   129,
     140,   141,   142,    87,    88,    60,   133,   146,    20,   148,
     149,    89,   159,    90,   161,   160,   151,   144,     0,    91,
      92,   155,   165,     1,     2,     3,     4,     5,   168,     6,
       7,     8,     9,    10,    11,   166,    12,   169,    13,    14,
      15,    16,   145,     0,    17,     0,    28,    28,   154,     0,
      18,    19,     0,   158,     0,     0,    20,     0,    21,     0,
      22,     0,    23,     0,     0,    24,     0,     1,     2,     3,
       4,    28,     0,     0,     0,     0,     0,     0,     0,    28,
      12,     0,    13,    14,    15,    16,     0,     0,    17,     0,
       0,     0,     0,     0,    18,    19,     0,     1,     2,     3,
       4,     0,    21,     0,    22,     0,    48,     0,     0,    24,
      12,     0,    13,    14,    15,    16,     0,     0,    17,     0,
       1,     2,     3,     4,    18,    19,     0,     0,     8,     0,
      60,     0,    21,    12,    22,    13,    14,    15,    16,    24,
       0,    17,     0,     1,     2,     3,     4,    18,    19,     0,
       0,     0,     0,     0,     0,    21,    12,    22,    13,    14,
      15,    16,    24,     0,    17,     0,     0,     0,     0,     0,
      18,    19,     0,     0,     0,     0,     0,     0,    21,    72,
      22,    73,     0,     0,     0,    24,     0,    74,    75,    76,
      77,    78,    79,    80,     0,     0,    81,    82,    83,    84,
       0,    72,     0,    73,     0,     0,     0,     0,   143,    74,
      75,    76,    77,    78,    79,    80,     0,     0,    81,    82,
      83,    84,    72,     0,    73,     0,     0,     0,     0,   109,
      74,    75,    76,    77,    78,    79,    80,     0,     0,    81,
      82,    83,    84,    72,     0,    73,     0,     0,     0,    85,
       0,    74,    75,    76,    77,    78,    79,    80,     0,     0,
      81,    82,    83,    84,    72,     0,    73,     0,     0,     0,
     105,     0,    74,    75,    76,    77,    78,    79,    80,     0,
       0,    81,    82,    83,    84,    72,     0,    73,     0,     0,
       0,   162,     0,    74,    75,    76,    77,    78,    79,    80,
       0,     0,    81,    82,    83,    84,    72,     0,    73,     0,
       0,   113,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,     0,    81,    82,    83,    84,    72,     0,    73,
       0,     0,   136,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,     0,    81,    82,    83,    84,    72,     0,
      73,     0,     0,   137,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,     0,    81,    82,    83,    84,    72,
       0,    73,   147,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,     0,    81,    82,    83,    84,
      72,     0,    73,   150,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,     0,    81,    82,    83,
      84,    72,   164,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    72,     0,    81,    82,
      83,    84,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,     0,    81,    82,    83,    84,    74,    75,    76,
      77,    78,    79,    80,     0,     0,    81,    82,    83,    84,
      74,    75,    76,    77,    78,    -1,    -1,     0,     0,    81,
      82,    83,    84
};

static const yytype_int16 yycheck[] =
{
       0,    45,   102,     4,   136,   137,    40,   141,    22,     9,
      40,     4,    12,    20,    18,    19,    38,    17,    40,    26,
      20,   155,    22,    40,    46,    18,    26,    18,    19,   161,
      25,    26,    27,    42,    42,     0,     4,   169,     4,    40,
     140,    55,    40,    43,    44,     4,    90,    40,    11,    37,
      39,    43,    45,    97,    23,    24,    25,    26,    27,    38,
      60,    40,    39,    32,    33,    34,    35,    46,    47,    41,
       4,     4,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    22,    86,     4,    42,    89,
      43,    41,    41,    30,    31,    36,    96,    40,    36,    41,
      40,    38,   146,    40,     8,   149,    41,   109,    -1,    46,
      47,    41,    41,     3,     4,     5,     6,     7,   162,     9,
      10,    11,    12,    13,    14,    41,    16,    41,    18,    19,
      20,    21,   112,    -1,    24,    -1,   136,   137,   138,    -1,
      30,    31,    -1,   143,    -1,    -1,    36,    -1,    38,    -1,
      40,    -1,    42,    -1,    -1,    45,    -1,     3,     4,     5,
       6,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,
      16,    -1,    18,    19,    20,    21,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,     3,     4,     5,
       6,    -1,    38,    -1,    40,    -1,    42,    -1,    -1,    45,
      16,    -1,    18,    19,    20,    21,    -1,    -1,    24,    -1,
       3,     4,     5,     6,    30,    31,    -1,    -1,    11,    -1,
      36,    -1,    38,    16,    40,    18,    19,    20,    21,    45,
      -1,    24,    -1,     3,     4,     5,     6,    30,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    16,    40,    18,    19,
      20,    21,    45,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    15,
      40,    17,    -1,    -1,    -1,    45,    -1,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,    33,    34,    35,
      -1,    15,    -1,    17,    -1,    -1,    -1,    -1,    44,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,    33,
      34,    35,    15,    -1,    17,    -1,    -1,    -1,    -1,    43,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    32,
      33,    34,    35,    15,    -1,    17,    -1,    -1,    -1,    42,
      -1,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      32,    33,    34,    35,    15,    -1,    17,    -1,    -1,    -1,
      42,    -1,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,    33,    34,    35,    15,    -1,    17,    -1,    -1,
      -1,    42,    -1,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    32,    33,    34,    35,    15,    -1,    17,    -1,
      -1,    41,    -1,    -1,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,    33,    34,    35,    15,    -1,    17,
      -1,    -1,    41,    -1,    -1,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,    33,    34,    35,    15,    -1,
      17,    -1,    -1,    41,    -1,    -1,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    32,    33,    34,    35,    15,
      -1,    17,    39,    -1,    -1,    -1,    -1,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,    33,    34,    35,
      15,    -1,    17,    39,    -1,    -1,    -1,    -1,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    32,    33,    34,
      35,    15,    37,    17,    -1,    -1,    -1,    -1,    -1,    23,
      24,    25,    26,    27,    28,    29,    15,    -1,    32,    33,
      34,    35,    -1,    -1,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,    33,    34,    35,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,    33,    34,    35,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    32,
      33,    34,    35
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     9,    10,    11,    12,
      13,    14,    16,    18,    19,    20,    21,    24,    30,    31,
      36,    38,    40,    42,    45,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    64,    67,    68,    69,    71,
      72,    73,    74,    40,    40,    40,     4,    40,    42,    52,
      42,    42,    52,     4,    52,    40,    56,    58,    56,    51,
      36,    52,    62,    63,    65,    66,    75,    52,    68,     4,
       0,    51,    15,    17,    23,    24,    25,    26,    27,    28,
      29,    32,    33,    34,    35,    42,    22,    30,    31,    38,
      40,    46,    47,    59,    60,    61,    38,    40,    46,    52,
      52,    63,    40,     4,    70,    42,    68,    37,    52,    43,
      39,    39,    43,    41,    41,    52,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      63,     4,     4,    52,    63,     4,    41,    41,    42,    70,
      43,    41,    41,    44,    62,    75,    40,    39,    41,    40,
      39,    41,    50,    50,    52,    41,    70,    67,    52,    63,
      63,     8,    42,    67,    37,    41,    41,    50,    63,    41,
      50
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    48,    49,    49,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    51,    51,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    53,    53,    53,    53,    53,    53,    53,    53,    54,
      55,    55,    55,    55,    55,    56,    56,    56,    56,    57,
      57,    57,    57,    58,    58,    58,    59,    59,    60,    61,
      62,    62,    63,    63,    64,    64,    65,    66,    67,    68,
      68,    69,    69,    69,    69,    69,    69,    70,    70,    71,
      71,    72,    73,    74,    74,    75,    75
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     2,     1,     1,     1,     1,     2,
       2,     1,     1,     1,     1,     2,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     3,     2,     2,     2,     2,     2,     2,     1,     3,
       1,     1,     1,     3,     1,     1,     2,     2,     1,     3,
       4,     3,     4,     4,     2,     6,     1,     1,     3,     5,
       1,     3,     1,     0,     3,     3,     1,     5,     3,     6,
       5,     1,     1,     1,     1,     1,     1,     1,     3,     5,
       7,     5,     9,     2,     3,     1,     3
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
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use ALPHA_YYerror or ALPHA_YYUNDEF. */
#define YYERRCODE ALPHA_YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
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
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
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
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
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

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = ALPHA_YYEMPTY; /* Cause a token to be read.  */

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

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
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
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

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
      yychar = yylex ();
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
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
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

  /* Discard the shifted token.  */
  yychar = ALPHA_YYEMPTY;
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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: %empty  */
#line 100 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("program", "");}
#line 1345 "alphaBisonParser.cpp"
    break;

  case 3: /* program: multi_stmt  */
#line 101 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("program", "multi_stmt");}
#line 1351 "alphaBisonParser.cpp"
    break;

  case 4: /* stmt: expr SEMI_COLON  */
#line 104 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","expr SEMI_COLON");}
#line 1357 "alphaBisonParser.cpp"
    break;

  case 5: /* stmt: ifstmt  */
#line 105 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","ifstmt");}
#line 1363 "alphaBisonParser.cpp"
    break;

  case 6: /* stmt: whilestmt  */
#line 106 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","whilestmt");}
#line 1369 "alphaBisonParser.cpp"
    break;

  case 7: /* stmt: forstmt  */
#line 107 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","forstmt");}
#line 1375 "alphaBisonParser.cpp"
    break;

  case 8: /* stmt: returnstmt  */
#line 108 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","returnstmt");}
#line 1381 "alphaBisonParser.cpp"
    break;

  case 9: /* stmt: BREAK SEMI_COLON  */
#line 109 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","BREAK SEMI_COLON");}
#line 1387 "alphaBisonParser.cpp"
    break;

  case 10: /* stmt: CONTINUE SEMI_COLON  */
#line 110 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","CONTINUE SEMI_COLON");}
#line 1393 "alphaBisonParser.cpp"
    break;

  case 11: /* stmt: block  */
#line 111 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","block");}
#line 1399 "alphaBisonParser.cpp"
    break;

  case 12: /* stmt: funcdef  */
#line 112 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","funcdef");}
#line 1405 "alphaBisonParser.cpp"
    break;

  case 13: /* stmt: SEMI_COLON  */
#line 113 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("stmt","SEMI_COLON");}
#line 1411 "alphaBisonParser.cpp"
    break;

  case 14: /* multi_stmt: stmt  */
#line 116 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("multi_stmt", "stmt");}
#line 1417 "alphaBisonParser.cpp"
    break;

  case 15: /* multi_stmt: stmt multi_stmt  */
#line 117 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("multi_stmt", "stmt multi_stmt");}
#line 1423 "alphaBisonParser.cpp"
    break;

  case 16: /* expr: assignexpr  */
#line 120 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "assignexpr");}
#line 1429 "alphaBisonParser.cpp"
    break;

  case 17: /* expr: expr PLUS expr  */
#line 121 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr PLUS expr");}
#line 1435 "alphaBisonParser.cpp"
    break;

  case 18: /* expr: expr MINUS expr  */
#line 122 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr MINUS expr");}
#line 1441 "alphaBisonParser.cpp"
    break;

  case 19: /* expr: expr MUL expr  */
#line 123 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr MUL expr");}
#line 1447 "alphaBisonParser.cpp"
    break;

  case 20: /* expr: expr DIV expr  */
#line 124 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr DIV expr");}
#line 1453 "alphaBisonParser.cpp"
    break;

  case 21: /* expr: expr MOD expr  */
#line 125 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr MOD expr");}
#line 1459 "alphaBisonParser.cpp"
    break;

  case 22: /* expr: expr GREATER_THAN expr  */
#line 126 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr GREATER_THAN expr");}
#line 1465 "alphaBisonParser.cpp"
    break;

  case 23: /* expr: expr GREATER_THAN_OR_EQUAL expr  */
#line 127 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr GREATER_THAN_OR_EQUAL expr");}
#line 1471 "alphaBisonParser.cpp"
    break;

  case 24: /* expr: expr LESS_THAN expr  */
#line 128 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr LESS_THAN expr");}
#line 1477 "alphaBisonParser.cpp"
    break;

  case 25: /* expr: expr LESS_THAN_OR_EQUAL expr  */
#line 129 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr LESS_THAN_OR_EQUAL expr");}
#line 1483 "alphaBisonParser.cpp"
    break;

  case 26: /* expr: expr EQUAL expr  */
#line 130 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr EQUAL expr ");}
#line 1489 "alphaBisonParser.cpp"
    break;

  case 27: /* expr: expr NOT_EQUAL expr  */
#line 131 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr NOT_EQUAL expr");}
#line 1495 "alphaBisonParser.cpp"
    break;

  case 28: /* expr: expr AND expr  */
#line 132 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr AND expr");}
#line 1501 "alphaBisonParser.cpp"
    break;

  case 29: /* expr: expr OR expr  */
#line 133 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "expr OR expr");}
#line 1507 "alphaBisonParser.cpp"
    break;

  case 30: /* expr: term  */
#line 134 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("expr", "term");}
#line 1513 "alphaBisonParser.cpp"
    break;

  case 31: /* term: LEFT_PARENTHESIS expr RIGHT_PARENTHESIS  */
#line 137 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("term", "LEFT_PARENTHESIS expr RIGHT_PARENTHESIS");}
#line 1519 "alphaBisonParser.cpp"
    break;

  case 32: /* term: MINUS expr  */
#line 138 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("term", "MINUS expr");}
#line 1525 "alphaBisonParser.cpp"
    break;

  case 33: /* term: NOT expr  */
#line 139 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("term", "NOT expr");}
#line 1531 "alphaBisonParser.cpp"
    break;

  case 34: /* term: PLUS_PLUS lvalue  */
#line 140 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("term", "PLUS_PLUS lvalue");}
#line 1537 "alphaBisonParser.cpp"
    break;

  case 35: /* term: lvalue PLUS_PLUS  */
#line 141 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("term", "lvalue PLUS_PLUS");}
#line 1543 "alphaBisonParser.cpp"
    break;

  case 36: /* term: MINUS_MINUS lvalue  */
#line 142 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("term", "MINUS_MINUS lvalue");}
#line 1549 "alphaBisonParser.cpp"
    break;

  case 37: /* term: lvalue MINUS_MINUS  */
#line 143 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("term", "lvalue MINUS_MINUS");}
#line 1555 "alphaBisonParser.cpp"
    break;

  case 38: /* term: primary  */
#line 144 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("term", "primary");}
#line 1561 "alphaBisonParser.cpp"
    break;

  case 39: /* assignexpr: lvalue ASSIGN expr  */
#line 147 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("assignexpr", "lvalue ASSIGN expr");}
#line 1567 "alphaBisonParser.cpp"
    break;

  case 40: /* primary: lvalue  */
#line 150 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("primary", "lvalue");}
#line 1573 "alphaBisonParser.cpp"
    break;

  case 41: /* primary: call  */
#line 151 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("primary", "call");}
#line 1579 "alphaBisonParser.cpp"
    break;

  case 42: /* primary: objectdef  */
#line 152 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("primary", "objectdef");}
#line 1585 "alphaBisonParser.cpp"
    break;

  case 43: /* primary: LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS  */
#line 153 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("primary", "LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS");}
#line 1591 "alphaBisonParser.cpp"
    break;

  case 44: /* primary: const  */
#line 154 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("primary", "const");}
#line 1597 "alphaBisonParser.cpp"
    break;

  case 45: /* lvalue: ID  */
#line 157 "alphaBisonParser.y"
                                                                                                        {DISPLAY_LOG("lvalue","ID");}
#line 1603 "alphaBisonParser.cpp"
    break;

  case 46: /* lvalue: LOCAL ID  */
#line 158 "alphaBisonParser.y"
                                                                                                        {DISPLAY_LOG("lvalue","LOCAL ID");}
#line 1609 "alphaBisonParser.cpp"
    break;

  case 47: /* lvalue: COLON_BLOCK ID  */
#line 159 "alphaBisonParser.y"
                                                                                                        {DISPLAY_LOG("lvalue","COLON_BLOCK ID");}
#line 1615 "alphaBisonParser.cpp"
    break;

  case 48: /* lvalue: member  */
#line 160 "alphaBisonParser.y"
                                                                                                        {DISPLAY_LOG("lvalue","member");}
#line 1621 "alphaBisonParser.cpp"
    break;

  case 49: /* member: lvalue DOT ID  */
#line 163 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("member","lvalue DOT ID");}
#line 1627 "alphaBisonParser.cpp"
    break;

  case 50: /* member: lvalue LEFT_BRACKET expr RIGHT_BRACKET  */
#line 164 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("member","lvalue LEFT_BRACKET expr RIGHT_BRACKET");}
#line 1633 "alphaBisonParser.cpp"
    break;

  case 51: /* member: call DOT ID  */
#line 165 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("member","call DOT ID");}
#line 1639 "alphaBisonParser.cpp"
    break;

  case 52: /* member: call LEFT_BRACKET expr RIGHT_BRACKET  */
#line 166 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("member","CALL LEFT_BRACKET expr RIGHT_BRACKET");}
#line 1645 "alphaBisonParser.cpp"
    break;

  case 53: /* call: call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS  */
#line 169 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("call","call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
#line 1651 "alphaBisonParser.cpp"
    break;

  case 54: /* call: lvalue callsuffix  */
#line 170 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("call","lvalue callsuffix");}
#line 1657 "alphaBisonParser.cpp"
    break;

  case 55: /* call: LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS  */
#line 172 "alphaBisonParser.y"
                                                                {DISPLAY_LOG("member","lvalue DOT ID");}
#line 1663 "alphaBisonParser.cpp"
    break;


#line 1667 "alphaBisonParser.cpp"

      default: break;
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
      yyerror (YY_("syntax error"));
    }

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
                      yytoken, &yylval);
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


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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
  yyerror (YY_("memory exhausted"));
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
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

