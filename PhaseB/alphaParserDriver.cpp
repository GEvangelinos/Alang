#include <iostream>
#include <filesystem>
#include <list>
#include "./GeneratedFiles/alphaBisonParser.hpp"
#include "./GeneratedFiles/alphaFlexScanner.hpp"
#include "./symbolTable.hpp"
#define INSIDE_BISON_FILE
#include "./alphaDefs.hpp"

Alpha::SymbolTable symbolTable;

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

int main(int argc, char **argv)
{
        setInputStream(argc, argv, &alpha_yyin);
        auto returnValue = alpha_yyparse();
        std::cout << "alpha__yyparse return value: " << returnValue << std::endl;
        symbolTable.printSymbolInsertionVector();
        symbolTable.printSyntaxErrorVector();
}