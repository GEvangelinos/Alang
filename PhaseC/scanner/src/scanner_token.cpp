#include "scanner/scanner_token.hpp"
#include <algorithm> // for transform
#include <cctype>    // for toupper
#include <cstring>   // for strcpy, strlen, size_t
#include <limits>    // for numeric_limits
#include <sstream>   // for basic_ostream, basic_stringstream, operator<<
#include <stdexcept> // for overflow_error, runtime_error, underflow_error

namespace alpha
{
        /*** STARTOF: Local to the file (static) definitions: ***/
        [[maybe_unused]] static std::string toUpperCase(const std::string &input)
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
                        throw std::underflow_error("validTokenCounter has reached its minimum value and will wrap-around.");
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
        char *TokenID::lastId = nullptr;

        TokenID::TokenID(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &idContent, const std::string __idName)
            : Token(lineNumber, tokenNumber, idContent), idName(__idName) {}

        char *TokenID::refreshLastId(const char *id)
        {
                // TODO in future driver.. just malloc a buffer, and write the same (grow only if needed..);
                if (!id)
                        throw std::runtime_error("Error in lexer (probably): `id` is null.");

                delete[] TokenID::lastId;                        /* delete[] is safe with nullptr too (in first initialization). */
                TokenID::lastId = new char[std::strlen(id) + 1]; /* +1 for '\0'. */
                std::strcpy(TokenID::lastId, id);
                return TokenID::lastId;
        }

        void TokenID::clearLastId() { delete[] TokenID::lastId; }

        std::string TokenID::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "ID"
                             << "\t"
                             << "\""
                             << this->idName
                             << "\"";
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenID definitions. ***/

        /*** STARTOF: class TokenComment definitions. ***/
        SourceLocation TokenComment::commentStartingLocation;

        void TokenComment::setStartingLocation(SourceLocation location)
        {
                TokenComment::commentStartingLocation = location;
        }

        SourceLocation TokenComment::getStartingLocation()
        {
                return TokenComment::commentStartingLocation;
        }

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

        /*** ENDOF: class TokenComment definitions. ***/

        /*** STARTOF: class TokenString definitions. ***/
        std::stringstream TokenString::stringAssemblingBuffer;
        SourceLocation TokenString::stringStartingLocation;

        void TokenString::setStartingLocation(SourceLocation location)
        {
                TokenString::stringStartingLocation = location;
        }

        SourceLocation TokenString::getStartingLocation()
        {
                return TokenString::stringStartingLocation;
        }

        /* TODO: FIXME: I AM UGLY AS FUCK... AT LEAST PUT replacement and replacee  into list and iterate.. jeez. */
        std::string TokenString::convertContentEscapesToASCII()
        {
                std::string str = stringAssemblingBuffer.str();
                std::string toReplace = "\\n";
                char replacement = '\n';
                size_t pos = 0;
                while ((pos = str.find(toReplace, pos)) != std::string::npos)
                {
                        str.replace(pos, toReplace.length(), 1, replacement);
                        pos += 1; // Move past the replacement
                }

                toReplace = "\\t";
                replacement = '\t';
                pos = 0;
                while ((pos = str.find(toReplace, pos)) != std::string::npos)
                {
                        str.replace(pos, toReplace.length(), 1, replacement);
                        pos += 1; // Move past the replacement
                }

                toReplace = "\\\\";
                replacement = '\\';
                pos = 0;
                while ((pos = str.find(toReplace, pos)) != std::string::npos)
                {
                        str.replace(pos, toReplace.length(), 1, replacement);
                        pos += 1; // Move past the replacement
                }

                toReplace = "\\\"";
                replacement = '\"';
                pos = 0;
                while ((pos = str.find(toReplace, pos)) != std::string::npos)
                {
                        str.replace(pos, toReplace.length(), 1, replacement);
                        pos += 1; // Move past the replacement
                }
                return str;
        }

        void TokenString::appendToAssemblingBuffer(std::string stringChunk)
        {
                stringAssemblingBuffer << stringChunk;
        }

        void TokenString::flushAssemblingBuffer()
        {
                stringAssemblingBuffer.str("");
                stringAssemblingBuffer.clear();
        }

        void TokenString::exportStringToken(char **cstring_addr)
        {
                std::string cpp_string = convertContentEscapesToASCII();
                *cstring_addr = new char[cpp_string.size() + 1]; // +1 for null terminator
                std::strcpy(*cstring_addr, cpp_string.c_str());
                TokenString::flushAssemblingBuffer();
        }

        TokenString::TokenString(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string stringContent)
            : Token(lineNumber, tokenNumber, stringContent)
        {
                /* Empty Constructor Body */
        }
        std::string TokenString::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "STRING"
                             << "\t"
                             << "\""
                             << Token::getTokenContent()
                             << "\"";
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenString definitions. ***/

        /*** STARTOF: class TokenInvalid definitions. ***/
        TokenInvalid::TokenInvalid(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string theInvalidToken)
            : Token(lineNumber, tokenNumber, theInvalidToken)
        {
                /* Empty Constructor Body */
        }

        std::string TokenInvalid::toString() const
        {
                std::stringstream stringBuffer;
                stringBuffer << Token::toString()
                             << "\t"
                             << "TOKEN_INVALID"
                             << "\t"
                             << Token::getTokenContent();
                return stringBuffer.str();
        }
        /*** ENDOF: class TokenInvalid definitions. ***/

} /* namespace Alpha */