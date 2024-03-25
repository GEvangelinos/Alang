#include <iostream>
#include <string>
#include <list>

#include "alphaDefs.hpp"
#include "alphaLang.hpp"
#include "alphaTokenCodes.hpp"
#include "alphaErrorCodes.hpp"

static void tokenParser(std::list<Alpha::Token *> &tokenList)
{
        struct Alpha::alpha_token_t currToken;
        int tokenCode = -1;
        while (tokenCode = alpha_yylex(&currToken))
        {
                if (tokenCode == Alpha::EOF_INSIDE_COMMENT)
                {
                        std::cout << "Token Code is " << tokenCode << std::endl;
                        return;
                }
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

static void printParsedTokens(const std::list<Alpha::Token *> &tokenList)
{
        std::cout << "\n--------------------Lexical Analysis--------------------\n";
        for (auto &token : tokenList)
                std::cout << token->toString() << std::endl;
        std::cout << "\n----------------End of Lexical Analysis-----------------\n";
}

int main()
{
        std::list<Alpha::Token *> tokenList;
        /* TODO: MAKE THE ABOVE AN ARRAYLIST FOR GOOD CACHE LOCALITY */
        tokenParser(tokenList);
        printParsedTokens(tokenList);
        return 0;
}