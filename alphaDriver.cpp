#include <iostream>
#include <string>
#include <list>

#include "alphaDefs.hpp"
#include "alphaLang.hpp"

static void tokenParser(std::list<Alpha::Token>& tokenList)
{
        struct Alpha::alpha_token_t curr_token;
        int ret_val = -1;
        while ((ret_val = alpha_yylex(&curr_token)) != 0)
        {
                std::cout << "LINE# " << curr_token.lineNumber << " RetVal == " << ret_val << std::endl;
        }
}


int main()
{
        std::list<Alpha::Token> tokenList;
        /* TODO: MAKE THE ABOVE AN ARRAYLIST FOR GOOD CACHE LOCALITY */
        tokenParser(tokenList);
        return 0;
}