#include <string>
namespace Alpha
{
        struct alpha_token_t
        {
                unsigned int lineNumber;
                unsigned int tokenNumber;
                std::string content;
                std::string type;
        };

        class Token
        {
        private:
                static unsigned int validTokenCounter;
                static void incrementValidTokenCounter(void);

        protected:
                int tokenNumber;
                int lineNumber;
                virtual void registerToken();
                virtual void printToken();

        public:
                Token() = default;
                static void exportToken(void *yylval, unsigned int __lineNumber, std::string __content, std::string __type);
                static unsigned int getValidTokenCounter(void);
        };

        class TokenKeyword : public Token
        {
        private:
                std::string keywordName;
                void registerToken();
                void printToken();
        };

        class TokenOperator : public Token
        {
        };

        class TokenNumber : public Token
        {
        };

        class TokerRealNumber : public Token
        {
        };

        class TokenString : public Token
        {
        };

        class TokenPunctuation : public Token
        {
        };

        class TokenId : public Token
        {
        };

        class TokenComment : public Token
        {
                /* Make sub class for each Different comment style. */
        };
}