#include <limits>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include "alphaLang.hpp"

namespace Alpha
{

        /*** STARTOF: Local to the file (static) definitions: ***/
        static std::string toUpperCase(const std::string &input)
        {
                std::string result = input;
                std::transform(result.begin(), result.end(), result.begin(),
                               [](unsigned char c) -> unsigned char
                               { return std::toupper(c); });
                return result;
        }
        /*** ENDOF: Local to the file (static) definitions: ***/

        /*** STARTOF: class Token definitions: ***/
        unsigned int Token::validTokenCounter = 0;

        void Token::exportToken(void *yylval, unsigned int __lineNumber, std::string __content, std::string __codeName)
        {
                struct alpha_token_t *casted_yylval = (struct alpha_token_t *)yylval;
                casted_yylval->lineNumber = __lineNumber;
                casted_yylval->tokenNumber = Token::getValidTokenCounter();
                casted_yylval->content = __content;
                casted_yylval->codeName = __codeName;
        }

        void Token::incrementValidTokenCounter(void)
        {
                if (validTokenCounter == std::numeric_limits<unsigned int>::max())
                        throw std::overflow_error("validTokenCounter has reached its maximum value and will wrap-around.");
                validTokenCounter++;
        }

        void Token::decrementValidTokenCounter(void)
        {
                if (validTokenCounter == std::numeric_limits<unsigned int>::min()) /* This should never happen. */
                        throw std::overflow_error("validTokenCounter has reached its minimum value and will wrap-around.");
                validTokenCounter--;
        }

        unsigned int Token::getValidTokenCounter(void)
        {
                return validTokenCounter;
        }

        Token::Token(const unsigned int __lineNumber, const unsigned int __tokenNumber, const std::string &__tokenContent)
            : lineNumber(__lineNumber), tokenNumber(__tokenNumber), tokenContent(__tokenContent)
        {
                incrementValidTokenCounter();
        }

        unsigned int Token::getTokenNumber(void) const
        {
                return this->tokenNumber;
        }

        unsigned int Token::getLineNumber(void) const
        {
                return this->lineNumber;
        }

        const std::string &Token::getTokenContent(void) const
        {
                return this->tokenContent;
        }

        std::string Token::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << this->lineNumber
                             << ":\t"
                             << "#"
                             << this->tokenNumber
                             << "\t"
                             << "\""
                             << this->tokenContent
                             << "\"";
                return stringBuffer.str();
        }
        /*** ENDOF: class Token definitions. ***/

        /*** STARTOF: class TokenKeyword definitions: ***/
        TokenKeyword::TokenKeyword(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &keywordContent, const std::string &keywordCodeName)
            : Token(lineNumber, tokenNumber, keywordContent), tokenCodeName(keywordCodeName)
        {
                /* Empty Constructor Body */
        }

        std::string TokenKeyword::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "KEYWORD"
                             << "\t"
                             << this->tokenCodeName;
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenKeyword definitions. ***/

        /*** STARTOF: class TokenOperator definitions: ***/
        TokenOperator::TokenOperator(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &operatorContent, const std::string &operatorCodeName)
            : Token(lineNumber, tokenNumber, operatorContent), tokenCodeName(operatorCodeName)
        {
                /* Empty Constructor Body */
        }

        std::string TokenOperator::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "OPERATOR"
                             << "\t"
                             << this->tokenCodeName;
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenOperator definitions. ***/

        /*** STARTOF: class TokenPunctuation definitions: ***/
        TokenPunctuation::TokenPunctuation(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &punctuationContent, const std::string &punctuationCodeName)
            : Token(lineNumber, tokenNumber, punctuationContent), tokenCodeName(punctuationCodeName)
        {
                /* Empty Constructor Body */
        }

        std::string TokenPunctuation::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "PUNCTUATION"
                             << "\t"
                             << this->tokenCodeName;
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenKeyword definitions. ***/

        /*** STARTOF: class TokenIntegerNumber definitions. ***/
        TokenIntegerNumber::TokenIntegerNumber(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &integerContent, const std::string __numberOfToken)
            : Token(lineNumber, tokenNumber, integerContent), numberOfToken(__numberOfToken)
        {
                /* Empty Constructor Body */
        }
        std::string TokenIntegerNumber::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "INTCONST"
                             << "\t"
                             << this->numberOfToken;
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenIntegerNumber definitions. ***/

        /*** STARTOF: class TokenRealNumber definitions. ***/
        TokenRealNumber::TokenRealNumber(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &integerContent, const std::string __numberOfToken)
            : Token(lineNumber, tokenNumber, integerContent), numberOfToken(__numberOfToken)
        {
                /* Empty Constructor Body */
        }
        std::string TokenRealNumber::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "REALCONST"
                             << "\t"
                             << this->numberOfToken;
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenIntegerNumber definitions. ***/

        /*** STARTOF: class TokenID definitions. ***/
        TokenID::TokenID(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &idContent, const std::string __idName)
            : Token(lineNumber, tokenNumber, idContent), idName(__idName)
        {
                /* Empty Constructor Body */
        }
        std::string TokenID::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "ID"
                             << "\t"
                             << this->idName;
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenID definitions. ***/

        /*** STARTOF: class TokenComment definitions. ***/
        int TokenComment::currentlyOpenedBlockComments = 0;
        std::vector<int> TokenComment::startLineOfBlockComments;
        bool TokenComment::inNestedCommentState = false;
        ;

        TokenComment::TokenComment(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &commentDescription, const std::string __commentType)
            : Token(lineNumber, tokenNumber, commentDescription), commentType(__commentType)
        {
        }

        std::string TokenComment::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "COMMENT"
                             << "\t"
                             << this->commentType;
                return stringBuffer.str();
        }

        void TokenComment::exportLineCommentToken(void *yylval, unsigned int __lineNumber)
        {
                struct alpha_token_t *casted_yylval = (struct alpha_token_t *)yylval;
                casted_yylval->lineNumber = __lineNumber;
                casted_yylval->tokenNumber = Token::getValidTokenCounter();
                casted_yylval->content = "";
                casted_yylval->codeName = "LINE_COMMENT";
        }
        void TokenComment::exportBlockCommentToken(void *yylval, unsigned int __startLine, unsigned int __endLine)
        {
                if (TokenComment::currentlyOpenedBlockComments <= 0)
                        throw std::runtime_error("Something went wrong and <currentlyOpenedBlockComments> is negative or zero.");

                struct alpha_token_t *casted_yylval = (struct alpha_token_t *)yylval;
                casted_yylval->lineNumber = __startLine;
                casted_yylval->tokenNumber = Token::getValidTokenCounter();

                std::stringstream stringBuffer;
                stringBuffer << __startLine << " - " << __endLine;
                casted_yylval->content = stringBuffer.str();

                casted_yylval->codeName = TokenComment::inNestedCommentState ? "NESTED_COMMENT" : "BLOCK_COMMENT";

                TokenComment::startLineOfBlockComments.pop_back();
                TokenComment::currentlyOpenedBlockComments--;
                if (TokenComment::currentlyOpenedBlockComments == 0)
                        inNestedCommentState = false;
        }
        void TokenComment::addStartLineOfBlockComment(const int startLine)
        {
                /* TODO: VERIFY up to LINE (add this field to TOKEN) and check startline is >= that it. */
                TokenComment::startLineOfBlockComments.push_back(startLine);
                TokenComment::currentlyOpenedBlockComments++;
                if (TokenComment::currentlyOpenedBlockComments > 1)
                        inNestedCommentState == true;
        }

        /*** ENDOF: class TokenComment definitions. ***/
}; /* Namespace Alpha */