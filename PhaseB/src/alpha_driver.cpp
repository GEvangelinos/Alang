#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <format>
#include "arguinator/arguinator.hpp"
#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_location.hpp"
#include "scanner/alpha_scanner_context.hpp"
#include "alpha_scanner.hpp"
#include "alpha_parser.hpp"
#include "core/alpha_error_tracker.hpp"
#include "core/alpha_types.hpp"
#include "misc/cli_color.h"

static std::streamsize filesize(std::ifstream &inputFile)
{
        inputFile.seekg(0, std::ios::end);
        std::streamsize filesize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        return filesize;
}

static void load_file_to_input_buffer(const std::string &filename, std::unique_ptr<char[]> &lexer_input_buffer)
{
        constexpr auto flex_buffer_end_padding = 2;
        std::ifstream inputFile(filename);
        if (!inputFile)
                throw std::runtime_error("Failed opening " + filename + " for reading.");

        const std::streamsize inputFileSize = filesize(inputFile);

        lexer_input_buffer = std::make_unique<char[]>(inputFileSize + flex_buffer_end_padding);
        if (!inputFile.read(lexer_input_buffer.get(), inputFileSize))
        {
                throw std::runtime_error("Failed reading file " + filename + ".");
        }
        lexer_input_buffer[inputFileSize] = lexer_input_buffer[inputFileSize + 1] = '\0';
}

static void configure_lexer_input(const std::string &input_filename,
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
                load_file_to_input_buffer(input_filename, lexer_input_buffer);
                *lexer_buffer_state = alpha_yy_scan_string(lexer_input_buffer.get());
                return;
        }
        /* If any check fails, default to STDIN. */
        alpha_yyin = stdin;
}

static Arguinator::Parser launch_cli_parser(int argc, const char *const *const argv)
{
        const std::string alpha_driver_description = "A tool for syntactical analysis on programming language Alpha"; // TODO: put in static header.
        Arguinator::Parser parser(argc, argv, alpha_driver_description);
        parser.set_flag("input-file")                                          // TODO: put in static header: MAYBE? or in constexpr here? :w
            .set_help("Use flag to provide the alpha file you want to parse.") // TODO: Put in static header
            .set_arity(1)
            .set_required();
        parser.parse_flags();
        return parser;
}

static void display_symbol_table(const Alpha::SymbolTable &st, const Alpha::LocationTracker &lt)
{
        std::cout << COLOR_ASCII_FG_BLUE;
        const auto &symbol_per_scope_vector = st.symbols_per_scope();
        for (Alpha::u32 scope = Alpha::k_global_scope;
             scope < symbol_per_scope_vector.size();
             scope++)
        {
                // if (symbol_per_scope_vector[scope].empty())
                //         continue;
                std::cout << std::format("--------------------     Scope #{}     --------------------\n", scope);
                for (auto symbol_ptr : symbol_per_scope_vector[scope])
                        std::cout << std::format(
                            "\"{}\" [{}] (line {}[B:{}, E:{}]) (scope {})\n",
                            symbol_ptr->name(),
                            Alpha::to_string(symbol_ptr->type()),
                            lt.find_symbol_line(symbol_ptr->location()),
                            symbol_ptr->location().first_index_,
                            symbol_ptr->location().last_index_,

                            symbol_ptr->scope());
                std::cout << std::endl;
        }
        std::cout << SGR_RESET;
}

static void launch_alpha_parser(const std::string &input_filename)
{
        Alpha::LexerCtx lexer_ctx(input_filename);
        Alpha::ParseCtx parse_ctx;
        Alpha::SymbolTable st;
        Alpha::ErrorTracker et;
        Alpha::LocationTracker lt;
        YY_BUFFER_STATE lexer_buffer_state;
        std::unique_ptr<char[]> lexer_input_buffer;
        try
        {
                configure_lexer_input(input_filename, lexer_input_buffer, &lexer_buffer_state);
        }
        catch (std::exception &e)
        {
                std::cerr << e.what() << std::endl;
                constexpr int severeError = 2;
                std::exit(severeError);
        }
        int returnValue = alpha_yyparse(lexer_ctx, parse_ctx, st, et, lt);
        std::cout << "alpha_yyparse return value: " << returnValue << std::endl; // TODO: UGLY AS FUCK FIX.
        display_symbol_table(st, lt);
        alpha_yy_delete_buffer(lexer_buffer_state);
}
#include <filesystem>
int main(int argc, char **argv)
{
        Arguinator::Parser cli_parser = launch_cli_parser(argc, argv);

        std::cout << std::filesystem::current_path() << std::endl;
        // TODO :put together
        const std::string &file_name = cli_parser("input-file");
        launch_alpha_parser(file_name);
}