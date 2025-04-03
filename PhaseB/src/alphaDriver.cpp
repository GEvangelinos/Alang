#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <format>
#include "alphaParser.hpp"
#include "alphaScanner.hpp"
#include "arguinator/arguinator.hpp"
#include "core/symbolTable.hpp"
#include "core/alphaDefs.hpp"

static std::streamsize fileSize(std::ifstream &inputFile)
{
        inputFile.seekg(0, std::ios::end);
        std::streamsize fileSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        return fileSize;
}
static void loadFileToInputBuffer(const std::string &filename, std::unique_ptr<char[]> &lexer_input_buffer)
{
        constexpr auto flex_buffer_end_padding = 2;
        std::ifstream inputFile(filename);
        if (!inputFile)
                throw std::runtime_error("Failed opening " + filename + " for reading.");

        const std::streamsize inputFileSize = fileSize(inputFile);

        lexer_input_buffer = std::make_unique<char[]>(inputFileSize + flex_buffer_end_padding);
        if (!inputFile.read(lexer_input_buffer.get(), inputFileSize))
        {
                throw std::runtime_error("Failed reading file " + filename + ".");
        }
        lexer_input_buffer[inputFileSize] = lexer_input_buffer[inputFileSize + 1] = '\0';
}

static void configureLexerInput(const std::string &input_filename,
                                std::unique_ptr<char[]> &lexer_input_buffer,
                                YY_BUFFER_STATE *lexer_buffer_state)
{
        if (!std::filesystem::is_regular_file(input_filename))
                std::cerr << "Redirecting to STDIN: " << input_filename << " is not a regular file." << std::endl;
        else if (!std::ifstream(input_filename).good())
                std::cerr << "Redirecting to STDIN: Cannot open " << input_filename << "." << std::endl;
        else
        {
                alpha_yyin = nullptr;
                loadFileToInputBuffer(input_filename, lexer_input_buffer);
                *lexer_buffer_state = alpha_yy_scan_string(lexer_input_buffer.get());
                return;
        }
        /* If any check fails, default to STDIN. */
        alpha_yyin = stdin;
}

static void manageParser(const std::string &input_filename,
                         Alpha::SymbolTable &symbol_table)
{
        YY_BUFFER_STATE lexer_buffer_state;
        std::unique_ptr<char[]> lexer_input_buffer;
        Alpha::InputBufferContext context(input_filename);
        try
        {
                configureLexerInput(input_filename, lexer_input_buffer, &lexer_buffer_state);
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

static Arguinator::Parser create_cli_parser(int argc, const char *const *const argv)
{
        const std::string alpha_driver_description = "DESCRIPTION OF ALPHA_DRIVER";
        // A tool for syntactical analysis on programming language Alpha
        Arguinator::Parser parser(argc, argv, alpha_driver_description);
        parser.set_flag("input-file")
            .set_help("Use flag to provide the alpha file you want to parse.")
            .set_arity(1)
            .set_required();
        parser.parse_flags();
        return parser;
}

int main(int argc, char **argv)
{
        Alpha::SymbolTable symbol_table;
        Arguinator::Parser cli_parser = create_cli_parser(argc, argv);
        std::cout << cli_parser["input-file"].at(0) << std::endl;
        manageParser(cli_parser("input-file"), symbol_table);
        symbol_table.printSymbolInsertionVector();
        symbol_table.printErrorVector();
}