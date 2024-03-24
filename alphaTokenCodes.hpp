#ifndef ALPHA_CODES_HPP
#define ALPHA_CODES_HPP

namespace Alpha
{
        constexpr int codePoolOffset = 1000;

        constexpr int keywordGroupCode = 1;
        constexpr int operatorGroupCode = 2;
        constexpr int punctuationGroupCode = 3;
        constexpr int numberTypeGroupCode = 4;
        constexpr int commentTypeGroupCode = 5;
        constexpr int idGroupCode = 6;

        constexpr int startingKeywordCode = keywordGroupCode * codePoolOffset;
        constexpr int startingOperatorCode = operatorGroupCode * codePoolOffset;
        constexpr int startingPunctuationCode = punctuationGroupCode * codePoolOffset;
        constexpr int startingNumberTypeCode = numberTypeGroupCode * codePoolOffset;
        constexpr int startingCommentTypeCode = commentTypeGroupCode * codePoolOffset;
        constexpr int idCode = idGroupCode * codePoolOffset;

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

        std::string keywordCodesToString(KeywordCodes keyCode)
        {
                switch(keyCode)
                {
                        case KeywordCodes::IF: return "IF";
                        case KeywordCodes::ELSE: return "ELSE";
                        case KeywordCodes::WHILE: return "WHILE";
                        case KeywordCodes::FOR: return "FOR";
                        case KeywordCodes::FUNCTION: return "FUNCTION";
                        case KeywordCodes::RETURN: return "RETURN";
                        case KeywordCodes::BREAK: return "BREAK";
                        case KeywordCodes::CONTINUE: return "CONTINUE";
                        case KeywordCodes::AND: return "AND";
                        case KeywordCodes::NOT: return "NOT";
                        case KeywordCodes::OR: return "OR";
                        case KeywordCodes::LOCAL: return "LOCAL";
                        case KeywordCodes::TRUE: return "TRUE";
                        case KeywordCodes::FALSE: return "FALSE";
                        case KeywordCodes::NIL: return "NIL";
                        default: throw std::runtime_error("Unknown keyword code was asked to be converted to string.");
                }
        }

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

        std::string operatorCodesToString(OperatorCodes operCode)
        {
                switch(operCode)
                {
                        case OperatorCodes::ASSIGN: return "ASSIGN";
                        case OperatorCodes::PLUS: return "PLUS";
                        case OperatorCodes::MINUS: return "MINUS";
                        case OperatorCodes::DIV: return "DIV";
                        case OperatorCodes::MOD: return "MOD";
                        case OperatorCodes::EQUAL: return "EQUAL";
                        case OperatorCodes::NOT_EQUAL: return "NOT_EQUAL";
                        case OperatorCodes::PLUS_PLUS: return "PLUS_PLUS";
                        case OperatorCodes::MINUS_MINUS: return "MINUS_MINUS";
                        case OperatorCodes::GREATER_THAN: return "GREATER_THAN";
                        case OperatorCodes::LESS_THAN: return "LESS_THAN";
                        case OperatorCodes::GREATER_THAN_OR_EQUAL: return "GREATER_THAN_OR_EQUAL";
                        case OperatorCodes::LESS_THAN_OR_EQUAL: return "LESS_THAN_OR_EQUAL";
                        default: throw std::runtime_error("Unknown operator code has asked to be converted to string.");
                }
        }

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

        std::string punctuationCodesToString(PunctuationCodes puncCode)
        {
                switch(puncCode)
                {
                        case PunctuationCodes::LEFT_BRACE: return "LEFT_BRACE";
                        case PunctuationCodes::RIGHT_BRACE: return "RIGHT_BRACE";
                        case PunctuationCodes::LEFT_BRACKET: return "LEFT_BRACKET";
                        case PunctuationCodes::RIGHT_BRACKET: return "RIGHT_BRACKET";
                        case PunctuationCodes::LEFT_PARENTHESIS: return "LEFT_PARENTHESIS";
                        case PunctuationCodes::RIGHT_PARENTHESIS: return "RIGHT_PARENTHESIS";
                        case PunctuationCodes::SEMI_COLON: return "SEMI_COLON";
                        case PunctuationCodes::COMMA: return "COMMA";
                        case PunctuationCodes::COLON: return "COLON";
                        case PunctuationCodes::COLON_BLOCK: return "COLON_BLOCK";
                        case PunctuationCodes::DOT: return "DOT";
                        case PunctuationCodes::DDOT: return "DDOT";
                        default: throw std::runtime_error("Unknown punctuation code has asked to be converted to string.");
                }
        }

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