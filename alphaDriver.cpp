#include <iostream>
#include <string>
#include <list>

#include "alphaDefs.hpp"
#include "alphaLang.hpp"
#include "alphaTokenCodes.hpp"

static void tokenParser(std::list<Alpha::Token *> &tokenList)
{
        struct Alpha::alpha_token_t currToken;
        int tokenCode = -1;
        while (tokenCode = alpha_yylex(&currToken))
        {
                const int groupCode = tokenCode / Alpha::codePoolOffset;
                Alpha::Token *newTokenTempPtr = NULL;
                switch (groupCode)
                {
                case Alpha::keywordGroupCode:
                        newTokenTempPtr = new Alpha::TokenKeyword(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
                        break;
                case Alpha::operatorGroupCode:
                        newTokenTempPtr = new Alpha::TokenOperator(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
                        break;
                case Alpha::punctuationGroupCode:
                        newTokenTempPtr = new Alpha::TokenPunctuation(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
                        break;
                case Alpha::integerNumberCode:
                        newTokenTempPtr = new Alpha::TokenIntegerNumber(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
                        break;
                case Alpha::realNumberCode:
                        newTokenTempPtr = new Alpha::TokenRealNumber(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
                        break;
                case Alpha::commentTypeGroupCode:
                        newTokenTempPtr = new Alpha::TokenComment(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
                        break;
                case 444444 /*TODO: STRING*/:
                        break;
                case Alpha::idCode:
                        newTokenTempPtr = new Alpha::TokenID(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
                        break;
                case Alpha::invalidCharacterCode:
                        break;
                default:
                        throw std::runtime_error("UN-FUCKING-KNOWN");
                }

                if (newTokenTempPtr == NULL)
                        throw std::runtime_error("Oops something went wrong and I also dereferenced NULL pointer, check it out!");
                tokenList.push_back(newTokenTempPtr);
        }
}

int main()
{
        std::list<Alpha::Token *> tokenList;
        /* TODO: MAKE THE ABOVE AN ARRAYLIST FOR GOOD CACHE LOCALITY */
        tokenParser(tokenList);

        for (auto &token : tokenList)
        {
                std::cout << token->toString() << std::endl;
        }
        return 0;
}