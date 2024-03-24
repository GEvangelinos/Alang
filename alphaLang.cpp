#include <limits>
#include <stdexcept>
#include "alphaLang.hpp"

namespace Alpha
{
        unsigned int Token::validTokenCounter = 0;

        void Token::incrementValidTokenCounter(void)
        {
                if (validTokenCounter == std::numeric_limits<unsigned int>::max())
                        throw std::overflow_error("validTokenCounter has reached its maximum value and will overflow.");
                validTokenCounter++;
        }

        unsigned int Token::getValidTokenCounter(void)
        {
                return validTokenCounter;
        }

        void Token::exportToken(void *yylval, unsigned int __lineNumber, std::string __content, std::string __type)
        {
                struct alpha_token_t *casted_yylval = (struct alpha_token_t *)yylval;
                casted_yylval->lineNumber = __lineNumber;
                casted_yylval->tokenNumber = getValidTokenCounter();
                incrementValidTokenCounter();
                casted_yylval->content = __content;
                casted_yylval->type = __type;
        }

}