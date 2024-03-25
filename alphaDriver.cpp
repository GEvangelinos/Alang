#include <iostream>
#include <string>
#include <list>

#include "alphaDefs.hpp"
#include "alphaLang.hpp"
#include "alphaTokenCodes.hpp"
#include "alphaExceptions.hpp"
#include "alphaFlexScanner.hpp"

static Alpha::Token *generateCodeBasedToken(const int groupCode, Alpha::alpha_token_t &currToken)
{
        switch (groupCode)
        {
        case Alpha::keywordGroupCode:
                return new Alpha::TokenKeyword(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::operatorGroupCode:
                return new Alpha::TokenOperator(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::punctuationGroupCode:
                return new Alpha::TokenPunctuation(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::integerNumberCode:
                return new Alpha::TokenIntegerNumber(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::realNumberCode:
                return new Alpha::TokenRealNumber(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::commentTypeGroupCode:
                return new Alpha::TokenComment(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case 444444 /*TODO: MAGIC NUMBER FOR STRING I WILL IMPLEMENT LATER SKIPP THIS CASE */:
                break;
        case Alpha::idCode:
                return new Alpha::TokenID(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::invalidCharacterCode:
                return new Alpha::TokenInvalid(currToken.lineNumber, currToken.tokenNumber, currToken.content);
        default:
                throw std::runtime_error("Failed to create token, potentially due to an unrecognized group code.");
        }
        return nullptr;
}

static void tokenParser(std::list<Alpha::Token *> &tokenList)
{
        struct Alpha::alpha_token_t currToken;
        try
        {
                for (int tokenCode = alpha_yylex(&currToken); tokenCode != ALPHA_YYLEX_EOF; tokenCode = alpha_yylex(&currToken))
                {
                        const int groupCode = tokenCode / Alpha::codePoolOffset;
                        Alpha::Token *newAlphaToken = generateCodeBasedToken(groupCode, currToken);
                        if (!newAlphaToken)
                                throw std::runtime_error("Code based token generator returned nullptr, A Token's constructor probably failed.");
                        tokenList.push_back(newAlphaToken);
                }
        }
        catch (Alpha::BlockCommentEOF &e)
        {
                std::cerr << e.what() << std::endl;
        }
        catch (std::runtime_error &e)
        {
                std::cerr << e.what() << std::endl;
        }
}

static void printParsedTokens(const std::list<Alpha::Token *> &tokenList)
{
        std::cout << "\n--------------------Lexical Analysis--------------------\n";
        for (auto &token : tokenList)
                std::cout << token->toString() << std::endl;
        std::cout << "\n----------------End of Lexical Analysis-----------------\n";
}

int main(int argc, char **argv)
{
        std::list<Alpha::Token *> tokenList;
        /* TODO: input to stdin or specified file. */
        /* TODO: MAKE THE ABOVE AN ARRAYLIST FOR GOOD CACHE LOCALITY */
        yyin = nullptr;
        tokenParser(tokenList);
        printParsedTokens(tokenList);
        /* TODO: export to stdout or specified file. */

        return 0;
}