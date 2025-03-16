#ifndef ALPHA_LANG_HPP
#define ALPHA_LANG_HPP

#include <string>
#include <vector>
namespace Alpha
{
        struct alpha_token_t
        {
        public:
                unsigned int lineNumber;
                unsigned int tokenNumber;
                std::string content;
                std::string codeName;
        };

        class Token
        {
        private:
                static unsigned int validTokenCounter;
                static void incrementValidTokenCounter(void);
                static void decrementValidTokenCounter(void);
                const unsigned int tokenNumber;
                const unsigned int lineNumber;
                const std::string tokenContent;

        protected:
                Token(const unsigned int __lineNumber, const unsigned int __tokenNumber, const std::string &__tokenContent);
                unsigned int getTokenNumber(void) const;
                unsigned int getLineNumber(void) const;
                const std::string &getTokenContent(void) const;

        public:
                ~Token() = default;
                static unsigned int getValidTokenCounter(void);
                virtual std::string toString() const = 0;
                static void exportToken(void *yylval, unsigned int __lineNumber, std::string __content, std::string __codeName);
        };

        class TokenKeyword final : public Token
        {
        private:
                const std::string tokenCodeName;

        public:
                TokenKeyword(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &keywordContent, const std::string &keywordCodeName);
                std::string toString() const override;
        };

        class TokenOperator final : public Token
        {
        private:
                const std::string tokenCodeName;

        public:
                TokenOperator(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &operatorContent, const std::string &operatorCodeName);
                std::string toString() const override;
        };

        class TokenPunctuation final : public Token
        {
        private:
                const std::string tokenCodeName;

        public:
                TokenPunctuation(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &punctuationContent, const std::string &punctuationCodeName);
                std::string toString() const override;
        };

        class TokenIntegerNumber final : public Token
        {
        private:
                const std::string numberOfToken;

        public:
                TokenIntegerNumber(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &integerContent, std::string __numberOfToken);
                std::string toString() const override;
        };

        class TokenRealNumber final : public Token
        {
        private:
                const std::string numberOfToken;

        public:
                TokenRealNumber(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &realContent, const std::string __numberOfToken);
                std::string toString() const override;
        };

        class TokenID final : public Token
        {
        private:
                const std::string idName;
                static char *lastId;

        public:
                TokenID(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &idContent, const std::string __idName);
                static char *refreshLastId(const char *alpha_yytext);
                std::string toString() const override;
        };

        class TokenString final : public Token
        {
        private:
                static std::stringstream stringAssemblingBuffer;
                static int stringStartingLineNumber;
                static void flushAssemblingBuffer();
                static std::string convertContentEscapesToASCII();

        public:
                static void setStringStartingLineNumber(int lineNumber);
                static int getStringStartingLineNumber();
                static void appendToAssemblingBuffer(std::string stringChunk);
                static void exportStringToken(char **unionStringLiteral);
                TokenString(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string stringContent);
                std::string toString() const;
        };

        class TokenComment final : public Token
        {
        private:
                std::string commentType;
                static int commentStartingLineNumber;

        public:
                static void setCommentStartingLineNumber(int lineNumber);
                static int getCommentStartingLineNumber();
                TokenComment(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &commentLines, const std::string commentType);
                std::string toString() const override;
        };

        class TokenInvalid final : public Token
        {
        public:
                TokenInvalid(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string theInvalidToken);
                std::string toString() const override;
        };
} /* namespace Alpha */

#endif /* ALPHA_LANG_HPP */