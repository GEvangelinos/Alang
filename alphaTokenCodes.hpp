#ifndef ALPHA_CODES_HPP
#define ALPHA_CODES_HPP

namespace Alpha
{
        constexpr int startingKeywordCode = 1000;
        constexpr int startingOperatorCode = 2000;
        constexpr int startingPunctuationCode = 3000;
        constexpr int startingNumberTypeCode = 4000;
        constexpr int startingCommentTypeCode = 5000;
        constexpr int idCode = 6000;

        enum class KeywordCodes : int
        {
                IF = startingKeywordCode, /* if */
                ELSE,                     /* else*/
                WHILE,                    /* while */
                FOR,                      /* for */
                FUNCTION,                 /* function */
                RETURN,                   /* return */
                BREAK,                    /* break */
                CONTINUE,                 /* continue */
                AND,                      /* and */
                NOT,                      /* not */
                OR,                       /* or */
                LOCAL,                    /* local */
                TRUE,                     /* true */
                FALSE,                    /* false */
                NIL,                      /* nil */
                __END_OF_KEYWORD_CODES__  /* END_OF_KEYWORDS NUMCODE*/
        };

        enum class OperatorCodes
        {
                ASSIGN = startingOperatorCode, /* = */
                PLUS,                          /* + */
                MINUS,                         /* - */
                MUL,                           /* * */
                DIV,                           /* / */
                MOD,                           /* % */
                EQUAL,                         /* == */
                NOT_EQUAL,                     /* != */
                PLUS_PLUS,                     /* ++ */
                MINUS_MINUS,                   /* -- */
                GREATER_THAN,                  /* > */
                LESS_THAN,                     /* < */
                GREATER_THAN_OR_EQUAL,         /* >= */
                LESS_THAN_OR_EQUAL,            /* <= */
                __END_OF_OPERATOR_CODES__      /* END_OF_OPERATORS NUMCODE*/
        };

        enum class PunctuationCodes
        {
                LEFT_BRACE = startingPunctuationCode, /* { */
                RIGHT_BRACE,                          /* } */
                LEFT_BRACKET,                         /* [ */
                RIGHT_BRACKET,                        /* ] */
                LEFT_PARENTHESIS,                     /* ( */
                RIGHT_PARENTHESIS,                    /* ) */
                SEMI_COLON,                           /* ; */
                COMMA,                                /* , */
                COLON,                                /* : */
                COLON_BLOCK,                          /* :: */
                DOT,                                  /* . */
                DDOT,                                 /* .. */
                __END_OF_PUNCTUATION_CODES__          /* END_OF_PUNCTUATION NUMCODE*/
        };

        enum class NumericTypeCodes
        {
                INTEGER_CONSTANT = startingNumberTypeCode,
                FLOAT_CONSTANT,
                __END_OF_NUMERIC_TYPE_CODES__
        };

        enum class CommentTypeCodes
        {
                LINE_COMMENT = startingCommentTypeCode,
                BLOCK_COMMENT,
                NESTED_COMMENT,
                __END_OF_COMMENT_TYPE_CODES__
        };
}
#endif /* ALPHA_CODES_HPP */