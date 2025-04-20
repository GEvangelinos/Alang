#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include "utils/format_adapter.hpp"
#include "arguinator/arguinator.hpp"
#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_location.hpp"
#include "scanner/alpha_scanner_context.hpp"
#include "alpha_scanner.hpp"
#include "alpha_parser.hpp"
#include "core/alpha_error_tracker.hpp"
#include "core/alpha_types.hpp"
#include "utils/cli_color.h"

static std::streamsize filesize(std::ifstream &inputFile)
{
        inputFile.seekg(0, std::ios::end);
        std::streamsize filesize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        return filesize;
}

static auto make_lexer_scan_buffer(std::ifstream &input_file)
{
        constexpr auto k_flex_EOF_padding = 2;

        if (!input_file)
                throw std::runtime_error("Invalid input file stream.");

        const auto input_file_size = filesize(input_file);
        const auto buffer_size = input_file_size + k_flex_EOF_padding;
        auto buffer = std::make_unique<char[]>(buffer_size);

        if (!input_file.read(buffer.get(), input_file_size))
                throw std::runtime_error("Failed reading input_file.");

        buffer[input_file_size] = buffer[input_file_size + 1] = '\0';
        return std::make_pair(std::move(buffer), buffer_size); // RVO
}

static std::ifstream open_alpha_source_file(const std::string &filename)
{
        if (!std::filesystem::is_regular_file(filename))
                throw std::invalid_argument(fmt_ns::format("{} is not a regular file.", filename));
        std::ifstream inputFile(filename);
        if (!inputFile)
                throw std::invalid_argument(fmt_ns::format("Failed opening {} for reading.", filename));
        return std::move(inputFile); // Should prefer RVO ?
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
                std::cout << fmt_ns::format("--------------------     Scope #{}     --------------------\n", scope);
                for (auto symbol_ptr : symbol_per_scope_vector[scope])
                        std::cout << fmt_ns::format(
                            "\"{}\" [{}] (line {}) (scope {})\n",
                            symbol_ptr->name(),
                            Alpha::to_string(symbol_ptr->type()),
                            lt.find_symbol_line(symbol_ptr->location()),
                            symbol_ptr->scope());
                std::cout << std::endl;
        }
        std::cout << SGR_RESET;
}

static void launch_alpha_parser(const std::string &source_file_name)
{
        std::ifstream alpha_source_file = open_alpha_source_file(source_file_name);
        Alpha::LexerCtx lexer_ctx(source_file_name);
        Alpha::ParseCtx parse_ctx;
        Alpha::SymbolTable st;
        Alpha::ErrorTracker et;
        Alpha::LocationTracker lt(filesize(alpha_source_file));
        auto lexer_buffer = make_lexer_scan_buffer(alpha_source_file);
        auto lexer_buffer_state = alpha_yy_scan_buffer(lexer_buffer.first.get(),lexer_buffer.second);
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
