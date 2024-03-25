#ifndef ALPHA_CODES_HPP
#define ALPHA_CODES_HPP

namespace Alpha
{
        constexpr int codePoolOffset = 1000;

        constexpr int endOfFileCode = 0;
        constexpr int keywordGroupCode = 1;
        constexpr int operatorGroupCode = 2;
        constexpr int punctuationGroupCode = 3;
        constexpr int commentTypeGroupCode = 4;
        constexpr int realNumberCode = 5;
        constexpr int integerNumberCode = 6;
        constexpr int idCode = 7;
        constexpr int stringLiteralCode = 8;
        constexpr int invalidCharacterCode = 10;

        constexpr int startingKeywordCode = keywordGroupCode * codePoolOffset;
        constexpr int startingOperatorCode = operatorGroupCode * codePoolOffset;
        constexpr int startingPunctuationCode = punctuationGroupCode * codePoolOffset;
        constexpr int startingCommentTypeCode = commentTypeGroupCode * codePoolOffset;
        constexpr int realNumberGroupCode = realNumberCode * codePoolOffset;
        constexpr int integerNumberGroupCode = integerNumberCode * codePoolOffset;
        constexpr int idGroupCode = idCode * codePoolOffset;
        constexpr int stringLiteralGroupCode = stringLiteralCode * codePoolOffset;
        constexpr int invalidCharacterGroupCode = invalidCharacterCode * codePoolOffset;

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

        enum class CommentTypeCodes
        {
                LINE = startingCommentTypeCode,
                BLOCK,
                NESTED,
                __END_OF_COMMENT_TYPE_CODES__
        };
}
#endif /* ALPHA_CODES_HPP */