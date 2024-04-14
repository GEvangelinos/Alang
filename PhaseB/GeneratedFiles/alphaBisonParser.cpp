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
        #include <list>
        #include <iostream>
        #include <stdexcept>
        #include "../GeneratedFiles/alphaFlexScanner.hpp"
        #include "../alphaDefs.hpp"
        bool isFunctionBlock = false;
        bool lvalueIsMember = false;

#line 91 "alphaBisonParser.cpp"

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
  YYSYMBOL_55_1 = 55,                      /* $@1  */
  YYSYMBOL_primary = 56,                   /* primary  */
  YYSYMBOL_lvalue = 57,                    /* lvalue  */
  YYSYMBOL_member = 58,                    /* member  */
  YYSYMBOL_call = 59,                      /* call  */
  YYSYMBOL_callsuffix = 60,                /* callsuffix  */
  YYSYMBOL_normcall = 61,                  /* normcall  */
  YYSYMBOL_methodcall = 62,                /* methodcall  */
  YYSYMBOL_expr_list = 63,                 /* expr_list  */
  YYSYMBOL_elist = 64,                     /* elist  */
  YYSYMBOL_objectdef = 65,                 /* objectdef  */
  YYSYMBOL_indexed = 66,                   /* indexed  */
  YYSYMBOL_indexedelem = 67,               /* indexedelem  */
  YYSYMBOL_block = 68,                     /* block  */
  YYSYMBOL_69_2 = 69,                      /* $@2  */
  YYSYMBOL_70_3 = 70,                      /* $@3  */
  YYSYMBOL_funcdef = 71,                   /* funcdef  */
  YYSYMBOL_72_4 = 72,                      /* $@4  */
  YYSYMBOL_73_5 = 73,                      /* $@5  */
  YYSYMBOL_74_6 = 74,                      /* $@6  */
  YYSYMBOL_const = 75,                     /* const  */
  YYSYMBOL_cs_ids = 76,                    /* cs_ids  */
  YYSYMBOL_77_7 = 77,                      /* $@7  */
  YYSYMBOL_idlist = 78,                    /* idlist  */
  YYSYMBOL_ifstmt = 79,                    /* ifstmt  */
  YYSYMBOL_whilestmt = 80,                 /* whilestmt  */
  YYSYMBOL_forstmt = 81,                   /* forstmt  */
  YYSYMBOL_returnstmt = 82,                /* returnstmt  */
  YYSYMBOL_indexedelem_list = 83           /* indexedelem_list  */
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
#define YYFINAL  71
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   590

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  48
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  36
/* YYNRULES -- Number of rules.  */
#define YYNRULES  96
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  180

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
static const yytype_int16 yyrline[] =
{
       0,   105,   105,   106,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   121,   122,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   142,   143,   144,   145,   150,   155,   160,   165,   170,
     169,   178,   179,   180,   181,   182,   185,   198,   215,   221,
     227,   228,   229,   230,   233,   234,   235,   239,   240,   243,
     246,   250,   251,   254,   255,   258,   259,   262,   265,   269,
     268,   280,   279,   294,   298,   292,   320,   316,   328,   329,
     330,   331,   332,   333,   336,   340,   340,   345,   346,   349,
     351,   355,   359,   363,   364,   367,   368
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
  "term", "assignexpr", "$@1", "primary", "lvalue", "member", "call",
  "callsuffix", "normcall", "methodcall", "expr_list", "elist",
  "objectdef", "indexed", "indexedelem", "block", "$@2", "$@3", "funcdef",
  "$@4", "$@5", "$@6", "const", "cs_ids", "$@7", "idlist", "ifstmt",
  "whilestmt", "forstmt", "returnstmt", "indexedelem_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-156)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-86)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     149,  -156,  -156,  -156,  -156,   -29,   -21,   -16,     1,   192,
     -24,   -12,   245,    32,  -156,  -156,  -156,   245,     2,     2,
      12,    98,   222,  -156,    34,    39,   149,  -156,   312,  -156,
    -156,  -156,   425,  -156,   -30,  -156,  -156,  -156,  -156,  -156,
    -156,  -156,  -156,   245,   245,   245,  -156,    41,  -156,   333,
    -156,  -156,  -156,  -156,     8,    40,   -15,   -30,   -15,   149,
      13,   245,   291,  -156,    14,    16,    15,  -156,   375,    11,
    -156,  -156,  -156,   245,   245,   245,   245,   245,   245,   245,
     245,   245,   245,   245,   245,   245,  -156,  -156,  -156,   245,
     245,    52,    53,    38,  -156,  -156,  -156,   245,   245,    58,
     396,   417,    21,    24,    22,  -156,    25,  -156,    26,    31,
    -156,   269,   245,  -156,  -156,    33,  -156,    30,   542,   529,
       8,     8,  -156,  -156,  -156,   555,   555,   247,   247,   247,
     247,   451,    45,  -156,    47,   245,   472,    49,  -156,   149,
     149,   245,    41,    28,  -156,    30,  -156,   245,  -156,  -156,
     245,  -156,  -156,   245,   514,  -156,  -156,    64,  -156,   354,
      50,    41,    56,   493,    54,    55,   149,   245,  -156,  -156,
    -156,  -156,  -156,  -156,  -156,    57,    56,   149,  -156,  -156
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,    80,    46,    78,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,    83,    81,     0,     0,     0,
      69,    64,     0,    13,     0,     0,    14,     3,     0,    30,
      16,    38,    41,    49,    42,    43,    11,    12,    45,     5,
       6,     7,     8,     0,     0,    64,    73,    88,    93,     0,
       9,    10,    33,    47,    32,     0,    34,     0,    36,     0,
       0,     0,    61,    63,     0,     0,    95,    67,     0,     0,
      48,     1,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,    35,    37,     0,
      64,     0,     0,     0,    55,    57,    58,     0,    64,     0,
       0,     0,     0,     0,    84,    87,     0,    94,     0,     0,
      72,     0,     0,    65,    66,     0,    31,    44,    28,    29,
      17,    18,    19,    20,    21,    26,    27,    22,    24,    23,
      25,     0,     0,    50,     0,     0,     0,     0,    52,     0,
       0,     0,    88,     0,    76,     0,    70,     0,    62,    96,
      64,    51,    59,    64,    40,    53,    54,    89,    91,     0,
       0,     0,     0,     0,     0,     0,     0,    64,    74,    86,
      77,    68,    56,    60,    90,     0,     0,     0,    75,    92
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -156,  -156,  -137,   -22,     0,  -156,  -156,  -156,  -156,    -5,
    -156,     9,  -156,  -156,  -156,   -19,   -44,  -156,  -156,  -156,
    -155,  -156,  -156,    -7,  -156,  -156,  -156,  -156,   -73,  -156,
     -48,  -156,  -156,  -156,  -156,   -10
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    25,    26,    27,    62,    29,    30,    93,    31,    32,
      33,    34,    94,    95,    96,    63,    64,    35,    65,    66,
      36,    59,    60,    37,   103,   176,   162,    38,   105,   143,
     106,    39,    40,    41,    42,    67
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      28,   102,   157,   158,    72,    46,     2,   170,    97,    49,
      98,    43,    52,    56,    58,    69,    99,    54,    50,    44,
      13,   178,    68,    89,    45,    90,    28,    57,    57,   174,
      51,    91,    92,    77,    78,    79,    53,   109,    70,    71,
     179,    47,    55,   100,   101,   104,   132,    24,   108,   -71,
     110,     8,   117,   113,   137,   114,   133,   134,   115,    28,
     135,   111,   138,   141,   142,   -85,   144,   145,   146,    61,
     150,   161,   166,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   152,   153,   169,   131,
     156,   168,    20,   148,   160,   172,   173,   136,   177,     0,
       0,     1,     2,     3,     4,   149,   164,     0,     0,   165,
       0,     0,     0,     0,    12,     0,    13,    14,    15,    16,
       0,     0,    17,   175,     0,     0,     0,     0,    18,    19,
       0,     0,     0,     0,    61,   154,    21,     0,    22,    28,
      28,   159,     0,    24,     0,     0,     0,   163,     0,     0,
       0,     0,     1,     2,     3,     4,     5,     0,     6,     7,
       8,     9,    10,    11,     0,    12,    28,    13,    14,    15,
      16,     0,     0,    17,     0,     0,     0,    28,     0,    18,
      19,     0,     0,     0,     0,    20,     0,    21,     0,    22,
       0,    23,     0,     0,    24,     1,     2,     3,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
      13,    14,    15,    16,     0,     0,    17,     0,     0,     0,
       0,     0,    18,    19,     0,     1,     2,     3,     4,     0,
      21,     0,    22,     8,    48,     0,     0,    24,    12,     0,
      13,    14,    15,    16,     0,     0,    17,     0,     1,     2,
       3,     4,    18,    19,     0,     0,     0,     0,     0,     0,
      21,    12,    22,    13,    14,    15,    16,    24,     0,    17,
      75,    76,    77,    78,    79,    18,    19,     0,     0,   -86,
     -86,   -86,   -86,    21,    73,    22,    74,     0,     0,     0,
      24,     0,    75,    76,    77,    78,    79,    80,    81,     0,
       0,    82,    83,    84,    85,     0,    73,     0,    74,     0,
       0,     0,     0,   147,    75,    76,    77,    78,    79,    80,
      81,     0,     0,    82,    83,    84,    85,    73,     0,    74,
       0,     0,     0,     0,   112,    75,    76,    77,    78,    79,
      80,    81,     0,     0,    82,    83,    84,    85,    73,     0,
      74,     0,     0,     0,    86,     0,    75,    76,    77,    78,
      79,    80,    81,     0,     0,    82,    83,    84,    85,    73,
       0,    74,     0,     0,     0,   107,     0,    75,    76,    77,
      78,    79,    80,    81,     0,     0,    82,    83,    84,    85,
      73,     0,    74,     0,     0,     0,   167,     0,    75,    76,
      77,    78,    79,    80,    81,     0,     0,    82,    83,    84,
      85,    73,     0,    74,     0,     0,   116,     0,     0,    75,
      76,    77,    78,    79,    80,    81,     0,     0,    82,    83,
      84,    85,    73,     0,    74,     0,     0,   139,     0,     0,
      75,    76,    77,    78,    79,    80,    81,   -39,     0,    82,
      83,    84,    85,     0,     0,    87,    88,     0,   140,     0,
       0,     0,     0,    89,     0,    90,    73,     0,    74,     0,
       0,    91,    92,     0,    75,    76,    77,    78,    79,    80,
      81,     0,     0,    82,    83,    84,    85,    73,     0,    74,
     151,     0,     0,     0,     0,    75,    76,    77,    78,    79,
      80,    81,     0,     0,    82,    83,    84,    85,    73,     0,
      74,   155,     0,     0,     0,     0,    75,    76,    77,    78,
      79,    80,    81,     0,     0,    82,    83,    84,    85,    73,
     171,    74,     0,     0,     0,     0,     0,    75,    76,    77,
      78,    79,    80,    81,    73,     0,    82,    83,    84,    85,
       0,     0,    75,    76,    77,    78,    79,    80,    81,     0,
       0,    82,    83,    84,    85,    75,    76,    77,    78,    79,
      80,    81,     0,     0,    82,    83,    84,    85,    75,    76,
      77,    78,    79,   -86,   -86,     0,     0,    82,    83,    84,
      85
};

static const yytype_int16 yycheck[] =
{
       0,    45,   139,   140,    26,     4,     4,   162,    38,     9,
      40,    40,    12,    18,    19,    22,    46,    17,    42,    40,
      18,   176,    22,    38,    40,    40,    26,    18,    19,   166,
      42,    46,    47,    25,    26,    27,     4,    59,     4,     0,
     177,    40,    40,    43,    44,     4,    90,    45,    55,    37,
      37,    11,    41,    39,    98,    39,     4,     4,    43,    59,
      22,    61,     4,    42,    40,    43,    41,    41,    37,    36,
      40,    43,     8,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    41,    40,   161,    89,
      41,    41,    36,   112,   142,    41,    41,    97,    41,    -1,
      -1,     3,     4,     5,     6,   115,   150,    -1,    -1,   153,
      -1,    -1,    -1,    -1,    16,    -1,    18,    19,    20,    21,
      -1,    -1,    24,   167,    -1,    -1,    -1,    -1,    30,    31,
      -1,    -1,    -1,    -1,    36,   135,    38,    -1,    40,   139,
     140,   141,    -1,    45,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    14,    -1,    16,   166,    18,    19,    20,
      21,    -1,    -1,    24,    -1,    -1,    -1,   177,    -1,    30,
      31,    -1,    -1,    -1,    -1,    36,    -1,    38,    -1,    40,
      -1,    42,    -1,    -1,    45,     3,     4,     5,     6,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      18,    19,    20,    21,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,     3,     4,     5,     6,    -1,
      38,    -1,    40,    11,    42,    -1,    -1,    45,    16,    -1,
      18,    19,    20,    21,    -1,    -1,    24,    -1,     3,     4,
       5,     6,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    16,    40,    18,    19,    20,    21,    45,    -1,    24,
      23,    24,    25,    26,    27,    30,    31,    -1,    -1,    32,
      33,    34,    35,    38,    15,    40,    17,    -1,    -1,    -1,
      45,    -1,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,    33,    34,    35,    -1,    15,    -1,    17,    -1,
      -1,    -1,    -1,    44,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,    33,    34,    35,    15,    -1,    17,
      -1,    -1,    -1,    -1,    43,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,    33,    34,    35,    15,    -1,
      17,    -1,    -1,    -1,    42,    -1,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    32,    33,    34,    35,    15,
      -1,    17,    -1,    -1,    -1,    42,    -1,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    32,    33,    34,    35,
      15,    -1,    17,    -1,    -1,    -1,    42,    -1,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    32,    33,    34,
      35,    15,    -1,    17,    -1,    -1,    41,    -1,    -1,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    32,    33,
      34,    35,    15,    -1,    17,    -1,    -1,    41,    -1,    -1,
      23,    24,    25,    26,    27,    28,    29,    22,    -1,    32,
      33,    34,    35,    -1,    -1,    30,    31,    -1,    41,    -1,
      -1,    -1,    -1,    38,    -1,    40,    15,    -1,    17,    -1,
      -1,    46,    47,    -1,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    32,    33,    34,    35,    15,    -1,    17,
      39,    -1,    -1,    -1,    -1,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,    33,    34,    35,    15,    -1,
      17,    39,    -1,    -1,    -1,    -1,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    32,    33,    34,    35,    15,
      37,    17,    -1,    -1,    -1,    -1,    -1,    23,    24,    25,
      26,    27,    28,    29,    15,    -1,    32,    33,    34,    35,
      -1,    -1,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,    33,    34,    35,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,    33,    34,    35,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    32,    33,    34,
      35
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     9,    10,    11,    12,
      13,    14,    16,    18,    19,    20,    21,    24,    30,    31,
      36,    38,    40,    42,    45,    49,    50,    51,    52,    53,
      54,    56,    57,    58,    59,    65,    68,    71,    75,    79,
      80,    81,    82,    40,    40,    40,     4,    40,    42,    52,
      42,    42,    52,     4,    52,    40,    57,    59,    57,    69,
      70,    36,    52,    63,    64,    66,    67,    83,    52,    71,
       4,     0,    51,    15,    17,    23,    24,    25,    26,    27,
      28,    29,    32,    33,    34,    35,    42,    30,    31,    38,
      40,    46,    47,    55,    60,    61,    62,    38,    40,    46,
      52,    52,    64,    72,     4,    76,    78,    42,    71,    51,
      37,    52,    43,    39,    39,    43,    41,    41,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    52,    64,     4,     4,    22,    52,    64,     4,    41,
      41,    42,    40,    77,    41,    41,    37,    44,    63,    83,
      40,    39,    41,    40,    52,    39,    41,    50,    50,    52,
      78,    43,    74,    52,    64,    64,     8,    42,    41,    76,
      68,    37,    41,    41,    50,    64,    73,    41,    68,    50
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    48,    49,    49,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    51,    51,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    53,    53,    53,    53,    53,    53,    53,    53,    55,
      54,    56,    56,    56,    56,    56,    57,    57,    57,    57,
      58,    58,    58,    58,    59,    59,    59,    60,    60,    61,
      62,    63,    63,    64,    64,    65,    65,    66,    67,    69,
      68,    70,    68,    72,    73,    71,    74,    71,    75,    75,
      75,    75,    75,    75,    76,    77,    76,    78,    78,    79,
      79,    80,    81,    82,    82,    83,    83
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     2,     1,     1,     1,     1,     2,
       2,     1,     1,     1,     1,     2,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     3,     2,     2,     2,     2,     2,     2,     1,     0,
       4,     1,     1,     1,     3,     1,     1,     2,     2,     1,
       3,     4,     3,     4,     4,     2,     6,     1,     1,     3,
       5,     1,     3,     1,     0,     3,     3,     1,     5,     0,
       4,     0,     3,     0,     0,     8,     0,     6,     1,     1,
       1,     1,     1,     1,     1,     0,     4,     1,     0,     5,
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
#line 105 "alphaBisonParser.y"
                                                                {displayLog("program", "");}
#line 1366 "alphaBisonParser.cpp"
    break;

  case 3: /* program: multi_stmt  */
#line 106 "alphaBisonParser.y"
                                                                {displayLog("program", "multi_stmt");}
#line 1372 "alphaBisonParser.cpp"
    break;

  case 4: /* stmt: expr SEMI_COLON  */
#line 109 "alphaBisonParser.y"
                                                                {displayLog("stmt","expr SEMI_COLON");}
#line 1378 "alphaBisonParser.cpp"
    break;

  case 5: /* stmt: ifstmt  */
#line 110 "alphaBisonParser.y"
                                                                {displayLog("stmt","ifstmt");}
#line 1384 "alphaBisonParser.cpp"
    break;

  case 6: /* stmt: whilestmt  */
#line 111 "alphaBisonParser.y"
                                                                {displayLog("stmt","whilestmt");}
#line 1390 "alphaBisonParser.cpp"
    break;

  case 7: /* stmt: forstmt  */
#line 112 "alphaBisonParser.y"
                                                                {displayLog("stmt","forstmt");}
#line 1396 "alphaBisonParser.cpp"
    break;

  case 8: /* stmt: returnstmt  */
#line 113 "alphaBisonParser.y"
                                                                {displayLog("stmt","returnstmt");}
#line 1402 "alphaBisonParser.cpp"
    break;

  case 9: /* stmt: BREAK SEMI_COLON  */
#line 114 "alphaBisonParser.y"
                                                                {displayLog("stmt","BREAK SEMI_COLON");}
#line 1408 "alphaBisonParser.cpp"
    break;

  case 10: /* stmt: CONTINUE SEMI_COLON  */
#line 115 "alphaBisonParser.y"
                                                                {displayLog("stmt","CONTINUE SEMI_COLON");}
#line 1414 "alphaBisonParser.cpp"
    break;

  case 11: /* stmt: block  */
#line 116 "alphaBisonParser.y"
                                                                {displayLog("stmt","block");}
#line 1420 "alphaBisonParser.cpp"
    break;

  case 12: /* stmt: funcdef  */
#line 117 "alphaBisonParser.y"
                                                                {displayLog("stmt","funcdef");}
#line 1426 "alphaBisonParser.cpp"
    break;

  case 13: /* stmt: SEMI_COLON  */
#line 118 "alphaBisonParser.y"
                                                                {displayLog("stmt","SEMI_COLON");}
#line 1432 "alphaBisonParser.cpp"
    break;

  case 14: /* multi_stmt: stmt  */
#line 121 "alphaBisonParser.y"
                                                                {displayLog("multi_stmt", "stmt");}
#line 1438 "alphaBisonParser.cpp"
    break;

  case 15: /* multi_stmt: stmt multi_stmt  */
#line 122 "alphaBisonParser.y"
                                                                {displayLog("multi_stmt", "stmt multi_stmt");}
#line 1444 "alphaBisonParser.cpp"
    break;

  case 16: /* expr: assignexpr  */
#line 125 "alphaBisonParser.y"
                                                                {displayLog("expr", "assignexpr");}
#line 1450 "alphaBisonParser.cpp"
    break;

  case 17: /* expr: expr PLUS expr  */
#line 126 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr PLUS expr");}
#line 1456 "alphaBisonParser.cpp"
    break;

  case 18: /* expr: expr MINUS expr  */
#line 127 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr MINUS expr");}
#line 1462 "alphaBisonParser.cpp"
    break;

  case 19: /* expr: expr MUL expr  */
#line 128 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr MUL expr");}
#line 1468 "alphaBisonParser.cpp"
    break;

  case 20: /* expr: expr DIV expr  */
#line 129 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr DIV expr");}
#line 1474 "alphaBisonParser.cpp"
    break;

  case 21: /* expr: expr MOD expr  */
#line 130 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr MOD expr");}
#line 1480 "alphaBisonParser.cpp"
    break;

  case 22: /* expr: expr GREATER_THAN expr  */
#line 131 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr GREATER_THAN expr");}
#line 1486 "alphaBisonParser.cpp"
    break;

  case 23: /* expr: expr GREATER_THAN_OR_EQUAL expr  */
#line 132 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr GREATER_THAN_OR_EQUAL expr");}
#line 1492 "alphaBisonParser.cpp"
    break;

  case 24: /* expr: expr LESS_THAN expr  */
#line 133 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr LESS_THAN expr");}
#line 1498 "alphaBisonParser.cpp"
    break;

  case 25: /* expr: expr LESS_THAN_OR_EQUAL expr  */
#line 134 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr LESS_THAN_OR_EQUAL expr");}
#line 1504 "alphaBisonParser.cpp"
    break;

  case 26: /* expr: expr EQUAL expr  */
#line 135 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr EQUAL expr ");}
#line 1510 "alphaBisonParser.cpp"
    break;

  case 27: /* expr: expr NOT_EQUAL expr  */
#line 136 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr NOT_EQUAL expr");}
#line 1516 "alphaBisonParser.cpp"
    break;

  case 28: /* expr: expr AND expr  */
#line 137 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr AND expr");}
#line 1522 "alphaBisonParser.cpp"
    break;

  case 29: /* expr: expr OR expr  */
#line 138 "alphaBisonParser.y"
                                                                {displayLog("expr", "expr OR expr");}
#line 1528 "alphaBisonParser.cpp"
    break;

  case 30: /* expr: term  */
#line 139 "alphaBisonParser.y"
                                                                {displayLog("expr", "term");}
#line 1534 "alphaBisonParser.cpp"
    break;

  case 31: /* term: LEFT_PARENTHESIS expr RIGHT_PARENTHESIS  */
#line 142 "alphaBisonParser.y"
                                                                {displayLog("term", "LEFT_PARENTHESIS expr RIGHT_PARENTHESIS");}
#line 1540 "alphaBisonParser.cpp"
    break;

  case 32: /* term: MINUS expr  */
#line 143 "alphaBisonParser.y"
                                                                {displayLog("term", "MINUS expr");}
#line 1546 "alphaBisonParser.cpp"
    break;

  case 33: /* term: NOT expr  */
#line 144 "alphaBisonParser.y"
                                                                {displayLog("term", "NOT expr");}
#line 1552 "alphaBisonParser.cpp"
    break;

  case 34: /* term: PLUS_PLUS lvalue  */
#line 145 "alphaBisonParser.y"
                                   {
                                displayLog("term", "PLUS_PLUS lvalue");
                                if ((yyvsp[0].unionLvalue) != nullptr && (((yyvsp[0].unionLvalue)->getType() == Alpha::SymbolType::LIBFUNC || (yyvsp[0].unionLvalue)->getType() == Alpha::SymbolType::USERFUNC)))
                                        symbolTable.registerSyntaxError(std::string("Operator ++  can not be used on function ") + (yyvsp[0].unionLvalue)->getName() , alpha_yylineno);
                        }
#line 1562 "alphaBisonParser.cpp"
    break;

  case 35: /* term: lvalue PLUS_PLUS  */
#line 150 "alphaBisonParser.y"
                                    {
                                displayLog("term", "lvalue PLUS_PLUS");
                                if ((yyvsp[-1].unionLvalue) != nullptr && (((yyvsp[-1].unionLvalue)->getType() == Alpha::SymbolType::LIBFUNC || (yyvsp[-1].unionLvalue)->getType() == Alpha::SymbolType::USERFUNC)))
                                        symbolTable.registerSyntaxError(std::string("Operator ++  can not be used on function ") + (yyvsp[-1].unionLvalue)->getName() , alpha_yylineno);
                        }
#line 1572 "alphaBisonParser.cpp"
    break;

  case 36: /* term: MINUS_MINUS lvalue  */
#line 155 "alphaBisonParser.y"
                                     {
                                displayLog("term", "MINUS_MINUS lvalue");
                                if ((yyvsp[0].unionLvalue) != nullptr && (((yyvsp[0].unionLvalue)->getType() == Alpha::SymbolType::LIBFUNC || (yyvsp[0].unionLvalue)->getType() == Alpha::SymbolType::USERFUNC)))
                                        symbolTable.registerSyntaxError(std::string("Operator --  can not be used on function ") + (yyvsp[0].unionLvalue)->getName() , alpha_yylineno);
                        }
#line 1582 "alphaBisonParser.cpp"
    break;

  case 37: /* term: lvalue MINUS_MINUS  */
#line 160 "alphaBisonParser.y"
                                     {
                                displayLog("term", "lvalue MINUS_MINUS");
                                if ((yyvsp[-1].unionLvalue) != nullptr && (((yyvsp[-1].unionLvalue)->getType() == Alpha::SymbolType::LIBFUNC || (yyvsp[-1].unionLvalue)->getType() == Alpha::SymbolType::USERFUNC)))
                                        symbolTable.registerSyntaxError(std::string("Operator --  can not be used on function ") + (yyvsp[-1].unionLvalue)->getName() , alpha_yylineno);
                        }
#line 1592 "alphaBisonParser.cpp"
    break;

  case 38: /* term: primary  */
#line 165 "alphaBisonParser.y"
                                                                {displayLog("term", "primary");}
#line 1598 "alphaBisonParser.cpp"
    break;

  case 39: /* $@1: %empty  */
#line 170 "alphaBisonParser.y"
                        {
                                if (!lvalueIsMember && (yyvsp[0].unionLvalue) != nullptr && ((yyvsp[0].unionLvalue)->getType() == Alpha::SymbolType::LIBFUNC || (yyvsp[0].unionLvalue)->getType() == Alpha::SymbolType::USERFUNC)) 
                                        symbolTable.registerSyntaxError(std::string((yyvsp[0].unionLvalue)->getName() + " is a function, can not assign to it."), alpha_yylineno);
                                lvalueIsMember = false;
                        }
#line 1608 "alphaBisonParser.cpp"
    break;

  case 40: /* assignexpr: lvalue $@1 ASSIGN expr  */
#line 175 "alphaBisonParser.y"
                                                         {displayLog("assignexpr", "lvalue ASSIGN expr");}
#line 1614 "alphaBisonParser.cpp"
    break;

  case 41: /* primary: lvalue  */
#line 178 "alphaBisonParser.y"
                                                                {displayLog("primary", "lvalue");}
#line 1620 "alphaBisonParser.cpp"
    break;

  case 42: /* primary: call  */
#line 179 "alphaBisonParser.y"
                                                                {displayLog("primary", "call");}
#line 1626 "alphaBisonParser.cpp"
    break;

  case 43: /* primary: objectdef  */
#line 180 "alphaBisonParser.y"
                                                                {displayLog("primary", "objectdef");}
#line 1632 "alphaBisonParser.cpp"
    break;

  case 44: /* primary: LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS  */
#line 181 "alphaBisonParser.y"
                                                                {displayLog("primary", "LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS");}
#line 1638 "alphaBisonParser.cpp"
    break;

  case 45: /* primary: const  */
#line 182 "alphaBisonParser.y"
                                                                {displayLog("primary", "const");}
#line 1644 "alphaBisonParser.cpp"
    break;

  case 46: /* lvalue: ID  */
#line 185 "alphaBisonParser.y"
                     {
                                displayLog("lvalue", "ID");
                                auto resultPair = symbolTable.lookUpSymbol((yyvsp[0].unionId));
                                if (resultPair.first == Alpha::OperationResult::SymbolNotFound)
                                {
                                        symbolTable.insertVariable((yyvsp[0].unionId), alpha_yylineno);
                                        resultPair = symbolTable.lookUpVariable((yyvsp[0].unionId));
                                        if (resultPair.first != Alpha::OperationResult::Success)
                                                throw std::runtime_error("Insertion of a variable failed after duplicate check.");
                                }
                                else
                                (yyval.unionLvalue) = resultPair.second;
                        }
#line 1662 "alphaBisonParser.cpp"
    break;

  case 47: /* lvalue: LOCAL ID  */
#line 198 "alphaBisonParser.y"
                           {
                                displayLog("lvalue","LOCAL ID");
                                Alpha::SymbolTableEntry *entry = symbolTable.lookUpCurrentScope((yyvsp[0].unionId));
                                if (symbolTable.isLibraryFunction((yyvsp[0].unionId)))
                                {
                                        symbolTable.registerSyntaxError(std::string((yyvsp[0].unionId)) + " shadows library function", alpha_yylineno);
                                        entry = nullptr;
                                }
                                else if (entry == nullptr) // if control reached here, it is not a LIBFUNC.
                                {
                                        symbolTable.insertVariable((yyvsp[0].unionId), alpha_yylineno);
                                        entry = symbolTable.lookUpVariable((yyvsp[0].unionId)).second;
                                        if (entry == nullptr)
                                                throw std::runtime_error("Insertion of a variable failed after duplicate check");
                                }
                                (yyval.unionLvalue) = entry;
                        }
#line 1684 "alphaBisonParser.cpp"
    break;

  case 48: /* lvalue: COLON_BLOCK ID  */
#line 215 "alphaBisonParser.y"
                                 {
                                displayLog("lvalue","COLON_BLOCK ID");
                                (yyval.unionLvalue) = symbolTable.lookUpGlobalScope((yyvsp[0].unionId));
                                if ((yyval.unionLvalue) == nullptr)
                                        symbolTable.registerSyntaxError(std::string("::") + std::string((yyvsp[0].unionId)) + " not found in global scope", alpha_yylineno);
                        }
#line 1695 "alphaBisonParser.cpp"
    break;

  case 49: /* lvalue: member  */
#line 221 "alphaBisonParser.y"
                        {
                                displayLog("lvalue","member");
                                lvalueIsMember = true;
                        }
#line 1704 "alphaBisonParser.cpp"
    break;

  case 50: /* member: lvalue DOT ID  */
#line 227 "alphaBisonParser.y"
                                                                {displayLog("member","lvalue DOT ID");}
#line 1710 "alphaBisonParser.cpp"
    break;

  case 51: /* member: lvalue LEFT_BRACKET expr RIGHT_BRACKET  */
#line 228 "alphaBisonParser.y"
                                                                {displayLog("member","lvalue LEFT_BRACKET expr RIGHT_BRACKET");}
#line 1716 "alphaBisonParser.cpp"
    break;

  case 52: /* member: call DOT ID  */
#line 229 "alphaBisonParser.y"
                                                                {displayLog("member","call DOT ID");}
#line 1722 "alphaBisonParser.cpp"
    break;

  case 53: /* member: call LEFT_BRACKET expr RIGHT_BRACKET  */
#line 230 "alphaBisonParser.y"
                                                                {displayLog("member","CALL LEFT_BRACKET expr RIGHT_BRACKET");}
#line 1728 "alphaBisonParser.cpp"
    break;

  case 54: /* call: call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS  */
#line 233 "alphaBisonParser.y"
                                                                {displayLog("call","call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
#line 1734 "alphaBisonParser.cpp"
    break;

  case 55: /* call: lvalue callsuffix  */
#line 234 "alphaBisonParser.y"
                                                                {displayLog("call","lvalue callsuffix");}
#line 1740 "alphaBisonParser.cpp"
    break;

  case 56: /* call: LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS  */
#line 236 "alphaBisonParser.y"
                                                                {displayLog("call","LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
#line 1746 "alphaBisonParser.cpp"
    break;

  case 57: /* callsuffix: normcall  */
#line 239 "alphaBisonParser.y"
                                                                {displayLog("callsuffix","normcall");}
#line 1752 "alphaBisonParser.cpp"
    break;

  case 58: /* callsuffix: methodcall  */
#line 240 "alphaBisonParser.y"
                                                                {displayLog("callsuffix","methodcall");}
#line 1758 "alphaBisonParser.cpp"
    break;

  case 59: /* normcall: LEFT_PARENTHESIS elist RIGHT_PARENTHESIS  */
#line 243 "alphaBisonParser.y"
                                                                {displayLog("normcall","LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
#line 1764 "alphaBisonParser.cpp"
    break;

  case 60: /* methodcall: DDOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS  */
#line 247 "alphaBisonParser.y"
                                                                {displayLog("methodcall","DDOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS");}
#line 1770 "alphaBisonParser.cpp"
    break;

  case 61: /* expr_list: expr  */
#line 250 "alphaBisonParser.y"
                                                                {displayLog("expr_list", "expr");}
#line 1776 "alphaBisonParser.cpp"
    break;

  case 62: /* expr_list: expr COMMA expr_list  */
#line 251 "alphaBisonParser.y"
                                                                {displayLog("expr_list", "expr COMMA expr_list");}
#line 1782 "alphaBisonParser.cpp"
    break;

  case 63: /* elist: expr_list  */
#line 254 "alphaBisonParser.y"
                                                                {displayLog("elist", "expr_list");}
#line 1788 "alphaBisonParser.cpp"
    break;

  case 64: /* elist: %empty  */
#line 255 "alphaBisonParser.y"
                                                                {displayLog("elist", "");}
#line 1794 "alphaBisonParser.cpp"
    break;

  case 65: /* objectdef: LEFT_BRACKET elist RIGHT_BRACKET  */
#line 258 "alphaBisonParser.y"
                                                                {displayLog("objectdef", "LEFT_BRACKET elist RIGHT_BRACKET");}
#line 1800 "alphaBisonParser.cpp"
    break;

  case 66: /* objectdef: LEFT_BRACKET indexed RIGHT_BRACKET  */
#line 259 "alphaBisonParser.y"
                                                                {displayLog("objectdef", "LEFT_BRACKET indexed RIGHT_BRACKET");}
#line 1806 "alphaBisonParser.cpp"
    break;

  case 67: /* indexed: indexedelem_list  */
#line 262 "alphaBisonParser.y"
                                                                {displayLog("indexed", "indexedelem_list");}
#line 1812 "alphaBisonParser.cpp"
    break;

  case 68: /* indexedelem: LEFT_BRACE expr COLON expr RIGHT_BRACE  */
#line 265 "alphaBisonParser.y"
                                                                {displayLog("indexedelem", "LEFT_BRACE expr COLON expr RIGHT_BRACE");}
#line 1818 "alphaBisonParser.cpp"
    break;

  case 69: /* $@2: %empty  */
#line 269 "alphaBisonParser.y"
                        {
                                symbolTable.incrementScope(isFunctionBlock); // isFunctionBlock is a bool: true, or false.
                                isFunctionBlock = false; // Reset flag.
                        }
#line 1827 "alphaBisonParser.cpp"
    break;

  case 70: /* block: LEFT_BRACE $@2 multi_stmt RIGHT_BRACE  */
#line 275 "alphaBisonParser.y"
                        {
                                symbolTable.decrementScope();
                                displayLog("block", "LEFT_BRACE multi_stmt RIGHT_BRACE");
                        }
#line 1836 "alphaBisonParser.cpp"
    break;

  case 71: /* $@3: %empty  */
#line 280 "alphaBisonParser.y"
                        {
                                // There might be FORMAL arguments to this function.
                                // And because member function incrementScope() declares them, we call it.
                                symbolTable.incrementScope(isFunctionBlock); // isFunctionBlock is a bool: true, or false.
                        }
#line 1846 "alphaBisonParser.cpp"
    break;

  case 72: /* block: LEFT_BRACE $@3 RIGHT_BRACE  */
#line 286 "alphaBisonParser.y"
                        {
                                symbolTable.decrementScope();
                                displayLog("block", "LEFT_BRACE RIGHT_BRACE");
                        }
#line 1855 "alphaBisonParser.cpp"
    break;

  case 73: /* $@4: %empty  */
#line 294 "alphaBisonParser.y"
                        {Alpha::Function::nameOfLastFunction = (yyvsp[0].unionId);}
#line 1861 "alphaBisonParser.cpp"
    break;

  case 74: /* $@5: %empty  */
#line 298 "alphaBisonParser.y"
                        {
                                Alpha::SymbolTableEntry *currentScopeEntry = symbolTable.lookUpCurrentScope(Alpha::Function::nameOfLastFunction);
                                if (symbolTable.isLibraryFunction(Alpha::Function::nameOfLastFunction))
                                        symbolTable.registerSyntaxError(std::string("Redefinition of library function ") + Alpha::Function::nameOfLastFunction + " is prohibited", alpha_yylineno);
                                else if (currentScopeEntry && currentScopeEntry->getType() == Alpha::SymbolType::USERFUNC)
                                        symbolTable.registerSyntaxError(std::string("Function") + Alpha::Function::nameOfLastFunction + " is already defined in this scope. Can not redifine.", alpha_yylineno);
                                else if (currentScopeEntry) // We found a symbol, and it was a LIBFUNC nor a USERFUNC, thus it is a variable
                                        symbolTable.registerSyntaxError(std::string(Alpha::Function::nameOfLastFunction) + " is already defined as a variable.", alpha_yylineno);
                                else
                                {
                                        symbolTable.insertFunction(Alpha::Function::nameOfLastFunction, alpha_yylineno, Alpha::SymbolType::USERFUNC, Alpha::Function::idList);
                                        isFunctionBlock = true;
                                }
                        }
#line 1880 "alphaBisonParser.cpp"
    break;

  case 75: /* funcdef: FUNCTION ID $@4 LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS $@5 block  */
#line 313 "alphaBisonParser.y"
                        {
                                displayLog("funcdef", "FUNCTION ID LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block");
                        }
#line 1888 "alphaBisonParser.cpp"
    break;

  case 76: /* $@6: %empty  */
#line 320 "alphaBisonParser.y"
                        {
                                symbolTable.insertNamelessFunction(alpha_yylineno, Alpha::SymbolType::USERFUNC, Alpha::Function::idList);
                                isFunctionBlock = true;
                        }
#line 1897 "alphaBisonParser.cpp"
    break;

  case 77: /* funcdef: FUNCTION LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS $@6 block  */
#line 325 "alphaBisonParser.y"
                        {displayLog("funcdef", "FUNCTION LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS block");}
#line 1903 "alphaBisonParser.cpp"
    break;

  case 78: /* const: INT_CONST  */
#line 328 "alphaBisonParser.y"
                                                                {displayLog("const", "INT_CONST");}
#line 1909 "alphaBisonParser.cpp"
    break;

  case 79: /* const: REAL_CONST  */
#line 329 "alphaBisonParser.y"
                                                                {displayLog("const", "REAL_CONST");}
#line 1915 "alphaBisonParser.cpp"
    break;

  case 80: /* const: STRING_LITERAL  */
#line 330 "alphaBisonParser.y"
                                                                {displayLog("const", "STRING_LITERAL");}
#line 1921 "alphaBisonParser.cpp"
    break;

  case 81: /* const: NIL  */
#line 331 "alphaBisonParser.y"
                                                                {displayLog("const", "NIL");}
#line 1927 "alphaBisonParser.cpp"
    break;

  case 82: /* const: TRUE  */
#line 332 "alphaBisonParser.y"
                                                                {displayLog("const", "TRUE");}
#line 1933 "alphaBisonParser.cpp"
    break;

  case 83: /* const: FALSE  */
#line 333 "alphaBisonParser.y"
                                                                {displayLog("const", "FALSE");}
#line 1939 "alphaBisonParser.cpp"
    break;

  case 84: /* cs_ids: ID  */
#line 336 "alphaBisonParser.y"
                     {
                        Alpha::Function::idList.push_back(std::string((yyvsp[0].unionId)));
                        displayLog("cs_ids", "ID");
                }
#line 1948 "alphaBisonParser.cpp"
    break;

  case 85: /* $@7: %empty  */
#line 340 "alphaBisonParser.y"
                     {
                        Alpha::Function::idList.push_back(std::string((yyvsp[0].unionId)));
                }
#line 1956 "alphaBisonParser.cpp"
    break;

  case 86: /* cs_ids: ID $@7 COMMA cs_ids  */
#line 342 "alphaBisonParser.y"
                                                                {displayLog("cs_ids", "ID COMMA cs_ids");}
#line 1962 "alphaBisonParser.cpp"
    break;

  case 87: /* idlist: cs_ids  */
#line 345 "alphaBisonParser.y"
                                                                {displayLog("idlist", "cs_ids");}
#line 1968 "alphaBisonParser.cpp"
    break;

  case 88: /* idlist: %empty  */
#line 346 "alphaBisonParser.y"
                                                                {displayLog("idlist", "");}
#line 1974 "alphaBisonParser.cpp"
    break;

  case 89: /* ifstmt: IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt  */
#line 350 "alphaBisonParser.y"
                                                                {displayLog("ifstmt", "IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt");}
#line 1980 "alphaBisonParser.cpp"
    break;

  case 90: /* ifstmt: IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt  */
#line 352 "alphaBisonParser.y"
                                                                {displayLog("ifstmt", "IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt");}
#line 1986 "alphaBisonParser.cpp"
    break;

  case 91: /* whilestmt: WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt  */
#line 356 "alphaBisonParser.y"
                                                                {displayLog("whilestmt", "WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt");}
#line 1992 "alphaBisonParser.cpp"
    break;

  case 92: /* forstmt: FOR LEFT_PARENTHESIS elist SEMI_COLON expr SEMI_COLON elist RIGHT_PARENTHESIS stmt  */
#line 360 "alphaBisonParser.y"
                                                                {displayLog("forstmt", "FOR LEFT_PARENTHESIS elist SEMI_CLON expr SEMI_CLON elist RIGHT_PARENTHESIS stmt");}
#line 1998 "alphaBisonParser.cpp"
    break;

  case 93: /* returnstmt: RETURN SEMI_COLON  */
#line 363 "alphaBisonParser.y"
                                                                {displayLog("returnstmt", "RETURN SEMI_COLON");}
#line 2004 "alphaBisonParser.cpp"
    break;

  case 94: /* returnstmt: RETURN expr SEMI_COLON  */
#line 364 "alphaBisonParser.y"
                                                                {displayLog("returnstmt", "RETURN expr SEMI_COLON");}
#line 2010 "alphaBisonParser.cpp"
    break;

  case 95: /* indexedelem_list: indexedelem  */
#line 367 "alphaBisonParser.y"
                                                                {displayLog("indexedelem_list", "indexedelem");}
#line 2016 "alphaBisonParser.cpp"
    break;

  case 96: /* indexedelem_list: indexedelem COMMA indexedelem_list  */
#line 368 "alphaBisonParser.y"
                                                                {displayLog("indexedelem_list", "indexedelem COMMA indexedelem_list");}
#line 2022 "alphaBisonParser.cpp"
    break;


#line 2026 "alphaBisonParser.cpp"

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

