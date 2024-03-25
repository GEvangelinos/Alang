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

        class TokenKeyword : public Token
        {
        private:
                const std::string tokenCodeName;

        public:
                TokenKeyword(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &keywordContent, const std::string &keywordCodeName);
                std::string toString() const override;
        };

        class TokenOperator : public Token
        {
        private:
                const std::string tokenCodeName;

        public:
                TokenOperator(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &operatorContent, const std::string &operatorCodeName);
                std::string toString() const override;
        };

        class TokenPunctuation : public Token
        {
        private:
                const std::string tokenCodeName;

        public:
                TokenPunctuation(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &punctuationContent, const std::string &punctuationCodeName);
                std::string toString() const override;
        };

        class TokenIntegerNumber : public Token
        {
        private:
                const std::string numberOfToken;

        public:
                TokenIntegerNumber(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &integerContent, std::string __numberOfToken);
                std::string toString() const override;
        };

        class TokenRealNumber : public Token
        {
        private:
                const std::string numberOfToken;

        public:
                TokenRealNumber(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &realContent, const std::string __numberOfToken);
                std::string toString() const override;
        };

        class TokenID : public Token
        {
        private:
                const std::string idName;

        public:
                TokenID(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &idContent, const std::string __idName);
                std::string toString() const override;
        };

        class TokenComment : public Token
        {
        private:
                std::string commentType;
                static bool inNestedState;
                static std::vector<int> startLineOfBlockComments;

        public:
                static void exportLineCommentToken(void *yylval, unsigned int __lineNumber);
                static void exportBlockCommentToken(void *yylval, unsigned int __endLine);
                static void addStartLineOfBlockComment(const int startLine);
                static bool isInNestedState();
                TokenComment(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string &commentLines, const std::string commentType);
                std::string toString() const override;
        };

        class TokenInvalid : public Token
        {
        public:
                TokenInvalid(const unsigned int lineNumber, const unsigned int tokenNumber, const std::string theInvalidToken);
                std::string toString() const override;
        };
} /* namespace Alpha */