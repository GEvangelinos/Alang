#define INSIDE_BISON_FILE
#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <format>
#include "alphaParser.hpp"
#include "alphaScanner.hpp"
#include "symbolTable.hpp"
#include "alphaDefs.hpp"

Alpha::SymbolTable symbolTable;

static std::streamsize fileSize(std::ifstream &inputFile)
{
        inputFile.seekg(0, std::ios::end);
        std::streamsize fileSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        return fileSize;
}

static void loadFileToInputBuffer(const std::string &filename, char **const inputBuffer)
{
        std::ifstream inputFile(filename);
        if (!inputFile)
                throw std::runtime_error("Failed opening " + filename + " for reading.");

        const std::streamsize inputFileSize = fileSize(inputFile);

        char *tempInputBuffer = new char[inputFileSize + 1];

        if (!inputFile.read(tempInputBuffer, inputFileSize))
        {
                delete[] tempInputBuffer;
                throw std::runtime_error("Failed reading file " + filename + ".");
        }
        tempInputBuffer[inputFileSize] = '\0';
        *inputBuffer = tempInputBuffer;
}

static void configureLexerInput(const int argc, const char *const *const argv, YY_BUFFER_STATE *lexer_buffer)
{
        const char *inputFilename = argc > 1 ? argv[1] : nullptr;

        /* Check if filename is provided, is not a directory, and is readable. */
        if (!inputFilename)
                std::cerr << "Redirecting to STDIN: No filename provided." << std::endl;
        else if (!std::filesystem::is_regular_file(inputFilename))
                std::cerr << "Redirecting to STDIN: " << inputFilename << " is not a regular file." << std::endl;
        else if (!std::ifstream(inputFilename).good())
                std::cerr << "Redirecting to STDIN: Cannot open " << inputFilename << "." << std::endl;
        else
        {
                alpha_yyin = nullptr;
                char *alpha_yybuf;
                loadFileToInputBuffer(argv[1], &alpha_yybuf);
                *lexer_buffer = alpha_yy_scan_string(alpha_yybuf);
                delete[] alpha_yybuf;
                return;
        }
        /* If any check fails, default to STDIN. */
        alpha_yyin = stdin;
}

static void manageParser(int argc, const char *const *const argv)
{
        YY_BUFFER_STATE lexer_buffer;
        try
        {
                configureLexerInput(argc, argv, &lexer_buffer);
        }
        catch (std::exception &e)
        {
                std::cerr << e.what() << std::endl;
                constexpr int severeError = 2;
                std::exit(severeError);
        }
        int returnValue = alpha_yyparse();
        std::cout << "alpha__yyparse return value: " << returnValue << std::endl;
        alpha_yy_delete_buffer(lexer_buffer);
}

int main(int argc, char **argv)
{
        manageParser(argc, argv);
        symbolTable.printSymbolInsertionVector();
        symbolTable.printErrorVector();
}