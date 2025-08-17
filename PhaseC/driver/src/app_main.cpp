#include <string>                    // for basic_string, string
#include "driver/alpha_driver.hpp"
#include "arguinator/arguinator.hpp" // for Flag, Parser
#include "utils/cli_color.h"

static constexpr char alpha_driver_description[] =
        "A tool for syntactical analysis on programming language Alpha";
static constexpr char flag_input_file[] = "input-file";
static constexpr char flag_export_symbol_table[] = "export-symbol-table";
static constexpr char flag_export_compile_errors[] = "export-compile-errors";
static constexpr char flag_export_quads[] = "export-quads";
static constexpr char flag_show_symbol_table[] = "show-symbol-table";
static constexpr char flag_show_parser_trace[] = "show-parser-trace";
static constexpr char flag_no_show_errors[] = "no-show-errors";
static constexpr char flag_show_quads[] = "show-quads";
static constexpr char flag_list_opts[] = "list-opts";

static Arguinator::Parser launch_cli_parser(int argc, const char *const *const argv)
{
    Arguinator::Parser parser(argc, argv, alpha_driver_description);
    parser.set_flag(flag_input_file)
          .set_arity(1)
          .set_required()
          .set_help("Use flag to provide the alpha file you want to parse.");
    parser.set_flag(flag_export_symbol_table)
          .set_arity(0)
          .set_help("If set, write the compiler's symbol table to a CSV file named "
               "<source_filename>.st.csv for external inspection.");
    parser.set_flag(flag_export_compile_errors)
          .set_arity(0)
          .set_help("If set, write the compiler's errors to a CSV file named "
               "<source_filename>.error.csv for external inspection.");
    parser.set_flag(flag_export_quads)
          .set_arity(0)
          .set_help("If set, write the compiler's generated QUADS to a file named"
               "<source_filename>.quads for external inspection");
    parser.set_flag(flag_show_symbol_table)
          .set_arity(0)
          .set_help("Pretty-prints the symbol table on console");
    parser.set_flag(flag_show_parser_trace)
          .set_arity(0) // no availble in opts
          .set_help("Pretty-prints a string message for each matched rule on "
               "parser's grammar");
    parser.set_flag(flag_no_show_errors)
          .set_arity(0)
          .set_help("Disables displaying of errors. (Mainly used for compiler' "
               "post validation, "
               "in automatic .tests)");
    parser.set_flag(flag_show_quads)
          .set_arity(0)
          .set_help("Pretty prints the quads on console");
    parser.set_flag(flag_list_opts)
          .set_arity(0)
          .set_help("lists of available optimizations, that can be enabled for IR "
               "generation");

    parser.parse_flags();

#if defined(OPTIMIZED_MODE) || defined(HATE_PYTHON_MODE)
    if (parser[flag_show_parser_trace].is_provided())
    {
        std::cout << COLOR_ASCII_BOLD_YELLOW
                << FMT::format("Flag --{} is disabled, due to OPTIMIZED or "
                               "HATE_PYTHON MODES build "
                               "parameters.\n"
                               "Either disable OPTIMIZED_MODE and "
                               "HATE_PYTHON_MODE or remove flag\n",
                               flag_show_parser_trace)
                << SGR_RESET << std::endl;
    }
#endif // OPTIMIZED_MODE OR HATE_PYTHON_MODE

    return parser; // NRVO
}

int main(const int argc, char **argv)
{
    const Arguinator::Parser cli_parser = launch_cli_parser(argc, argv);
    std::unique_ptr<alpha::Driver> driver;
    try
    {
        const std::string &source_filename = cli_parser[flag_input_file].get_input();
        const bool show_parser_trace = cli_parser[flag_show_parser_trace].is_provided();
        driver = std::make_unique<alpha::Driver>(source_filename, show_parser_trace);
    }
    catch (Arguinator::CLIHelp) { return 0; }

    driver->run();

    if (cli_parser[flag_export_symbol_table].is_provided())
        driver->export_symbol_table();
    if (cli_parser[flag_export_compile_errors].is_provided())
        driver->export_compile_errors();
    if (cli_parser[flag_export_quads].is_provided())
        driver->export_quads();
    if (cli_parser[flag_show_symbol_table].is_provided())
        driver->show_symbol_table();
    if (cli_parser[flag_show_quads].is_provided())
        driver->show_quads();
    if (!cli_parser[flag_no_show_errors].is_provided()) // Used by regression-test tool.
        driver->show_compile_issues();

    return driver->ok() ? 0 : 1;
}
