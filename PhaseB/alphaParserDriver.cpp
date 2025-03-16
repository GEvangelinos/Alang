#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <format>
#include "./GeneratedFiles/alphaBisonParser.hpp"
#include "./GeneratedFiles/alphaFlexScanner.hpp"
#include "./symbolTable.hpp"
#define INSIDE_BISON_FILE
#include "./alphaDefs.hpp"

Alpha::SymbolTable symbolTable;

static inline bool existsArgumentForFileName(const int argc) noexcept
{
        return argc > 1;
}

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

static void configureLexerInput(const int argc, const char *const *const argv, FILE **const alpha_yyin, char **alpha_yybuf)
{
        if (!existsArgumentForFileName(argc))
        {
                std::cerr << "Redirecting input to STDIN: Argument for filename is missing." << std::endl;
                *alpha_yyin = stdin;
                return;
        }

        if (std::filesystem::is_directory(argv[1]))
        {
                std::cerr << "Redirecting input to STDIN: Argument for filename corresponds to a directory." << std::endl;
                *alpha_yyin = stdin;
                return;
        }

        if (!std::ifstream(argv[1]).good())
        {
                std::cerr << "Redirecting input to STDIN: Unable to open file " << argv[1] << " for reading." << std::endl;
                *alpha_yyin = stdin;
                return;
        }
        *alpha_yyin = nullptr;
        loadFileToInputBuffer(argv[1], alpha_yybuf);
}

int main(int argc, char **argv)
{
        char *fileBuffer;
        try
        {
                configureLexerInput(argc, argv, &alpha_yyin, &fileBuffer);
        }
        catch (std::exception &e)
        {
                std::cerr << e.what() << std::endl;
                constexpr int severeError = 2;
                std::exit(severeError);
        }

        // YY_BUFFER_STATE buf = alpha_yy_scan_string(input);
        auto returnValue = alpha_yyparse();
        std::cout << "alpha__yyparse return value: " << returnValue << std::endl;
        symbolTable.printSymbolInsertionVector();
        symbolTable.printErrorVector();
}