#include <filesystem>
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
        case Alpha::realNumberGroupCode:
                return new Alpha::TokenRealNumber(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::integerNumberGroupCode:
                return new Alpha::TokenIntegerNumber(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::stringLiteralGroupCode:
                return new Alpha::TokenString(currToken.lineNumber, currToken.tokenNumber, currToken.content);
        case Alpha::idGroupCode:
                return new Alpha::TokenID(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::commentTypeGroupCode:
                return new Alpha::TokenComment(currToken.lineNumber, currToken.tokenNumber, currToken.content, currToken.codeName);
        case Alpha::invalidCharacterGroupCode:
                return new Alpha::TokenInvalid(currToken.lineNumber, currToken.tokenNumber, currToken.content);
        default:
                throw std::runtime_error("Failed to create token, potentially due to an unrecognized group code.");
        }
        return nullptr;
}

static void tokenParser(std::list<Alpha::Token *> &tokenList, std::string &possibleErrorMessage)
{
        struct Alpha::alpha_token_t currToken;
        try
        {
                for (int tokenCode = alpha_yylex(&currToken); tokenCode != ALPHA_YYLEX_EOF; tokenCode = alpha_yylex(&currToken))
                {
                        std::cout << "TokenCode == " << tokenCode << std::endl;
                        const int groupCode = tokenCode / Alpha::codePoolOffset;
                        Alpha::Token *newAlphaToken = generateCodeBasedToken(groupCode, currToken);
                        if (!newAlphaToken)
                                throw std::runtime_error("Code based token generator returned nullptr, A Token's constructor probably failed.");
                        tokenList.push_back(newAlphaToken);
                }
        }
        catch (Alpha::BlockCommentEOF &e)
        {
                possibleErrorMessage = e.what();
        }
        /* TODO: BOTH classes of exception do  the same so catch the abstract one... and make abstract... well abstract! */
        catch (Alpha::StringEOF &e)
        {
                possibleErrorMessage = e.what();
        }
        catch (std::runtime_error &e)
        {
                std::cerr << "Atypical error must have occured: \n"
                          << e.what() << std::endl;
        }
}

static void exportParsedTokens(const std::list<Alpha::Token *> &tokenList, FILE *alpha_yyout)
{
        fprintf(alpha_yyout, "\n--------------------Lexical Analysis--------------------\n");
        for (auto token : tokenList)
                fprintf(alpha_yyout, "%s\n", token->toString().c_str());
        fprintf(alpha_yyout, "\n----------------End of Lexical Analysis-----------------\n");
}

/* We open the file with C-style practices as FLEX's yyin is FILE pointer. */
static void setInputStream(const int argc, char **const argv, FILE **const alpha_yyin)
{
        *alpha_yyin = stdin; /* Default input stream. */
        if (argc > 1)
        {
                FILE *fptr = fopen(argv[1], "r");
                if (fptr == NULL)
                        std::cerr << "Failed opening input file for reading. Reading from STDIN." << std::endl;
                else if (std::filesystem::is_directory(argv[1]))
                {

                        std::cerr << "Given input file turned out to be a directory, oops..." << std::endl;
                        std::cerr << "Redirecting input to STDIN..." << std::endl;
                        fclose(fptr);
                }
                else
                        *alpha_yyin = fptr;
        }
}

/* We open the file with C-style practices as FLEX's yyin is FILE pointer. */
static void setOutputStream(const int argc, char **const argv, FILE **const alpha_yyout)
{
        *alpha_yyout = stdout; /* Default input stream. */
        if (argc > 2)
        {
                FILE *fptr = fopen(argv[2], "w");
                if (fptr == NULL)
                        std::cerr << "Failed opening output file for writitng. Writing to STDOUT." << std::endl;
                else
                        *alpha_yyout = fptr;
        }
}

static void exportPossibleErrorMessage(std::string errorMessage, FILE *alpha_yyout)
{
        if (errorMessage.empty()) /* No error message exiting. */
                return;
        fprintf(alpha_yyout, "%s\n", errorMessage.c_str());
}

int main(int argc, char **argv)
{
        /* TODO: MAKE THE ABOVE AN ARRAYLIST FOR GOOD CACHE LOCALITY */
        std::list<Alpha::Token *> tokenList;
        setInputStream(argc, argv, &yyin);
        setOutputStream(argc, argv, &yyout);
        std::string errorMessage; /* For possibly caught errors. */
        tokenParser(tokenList, errorMessage);
        exportParsedTokens(tokenList, yyout);
        exportPossibleErrorMessage(errorMessage, yyout);
        return 0;
}