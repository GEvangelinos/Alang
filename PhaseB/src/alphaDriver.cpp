#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <format>
#include "alphaParser.hpp"
#include "alphaScanner.hpp"
#include "core/symbolTable.hpp"
#include "core/alphaDefs.hpp"

#define FLEX_BUFFER_END_PADDING 2

static std::streamsize fileSize(std::ifstream &inputFile)
{
        inputFile.seekg(0, std::ios::end);
        std::streamsize fileSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        return fileSize;
}
static void loadFileToInputBuffer(const std::string &filename, std::unique_ptr<char[]> &lexer_input_buffer)
{
        std::ifstream inputFile(filename);
        if (!inputFile)
                throw std::runtime_error("Failed opening " + filename + " for reading.");

        const std::streamsize inputFileSize = fileSize(inputFile);

        lexer_input_buffer = std::make_unique<char[]>(inputFileSize + FLEX_BUFFER_END_PADDING);
        if (!inputFile.read(lexer_input_buffer.get(), inputFileSize))
        {
                throw std::runtime_error("Failed reading file " + filename + ".");
        }
        lexer_input_buffer[inputFileSize] = lexer_input_buffer[inputFileSize + 1] = '\0';
}

static void configureLexerInput(const int argc,
                                const char *const *const argv,
                                std::unique_ptr<char[]> &lexer_input_buffer,
                                YY_BUFFER_STATE *lexer_buffer_state)
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
                loadFileToInputBuffer(argv[1], lexer_input_buffer);
                *lexer_buffer_state = alpha_yy_scan_string(lexer_input_buffer.get());
                return;
        }
        /* If any check fails, default to STDIN. */
        alpha_yyin = stdin;
}

static void manageParser(int argc,
                         const char *const *const argv,
                         Alpha::SymbolTable &symbol_table,
                         Alpha::InputBufferContext &context)
{
        YY_BUFFER_STATE lexer_buffer_state;
        std::unique_ptr<char[]> lexer_input_buffer;
        try
        {
                configureLexerInput(argc, argv, lexer_input_buffer, &lexer_buffer_state);
        }
        catch (std::exception &e)
        {
                std::cerr << e.what() << std::endl;
                constexpr int severeError = 2;
                std::exit(severeError);
        }
        int returnValue = alpha_yyparse(symbol_table, context);
        std::cout << "alpha_yyparse return value: " << returnValue << std::endl;
        alpha_yy_delete_buffer(lexer_buffer_state);
}

int main(int argc, char **argv)
{
        Alpha::SymbolTable symbol_table;
        Alpha::InputBufferContext context;
        manageParser(argc, argv, symbol_table, context);
        symbol_table.printSymbolInsertionVector();
        symbol_table.printErrorVector();
}