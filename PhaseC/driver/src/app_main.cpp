#include <string>                    // for basic_string, string
#include "arguinator/arguinator.hpp" // for Flag, Parser
#include "driver/alpha_driver.hpp"
#include <driver/compilation_options.hpp>


static constexpr char alpha_driver_description[] =
        "A tool for syntactical analysis on programming language Alpha";

#if defined(OPTIMIZED_MODE) || defined(HATE_PYTHON_MODE)
#include "utils/cli_color.h"
#endif
static arguinator::Parser launch_cli_parser(const int argc, const char *const *const argv)
{
    arguinator::Parser parser(argc, argv, alpha_driver_description);

    for (const alpha::OptionSpec &os: alpha::option_specs)
    {
        auto &flag = parser.set_flag(os.name)
                           .set_arity(os.arity)
                           .set_help(os.help);
        if (os.required)
            flag.set_required();
    }

    parser.parse_flags();

    #if defined(OPTIMIZED_MODE) || defined(HATE_PYTHON_MODE)
    if (parser[alpha::flag_show_parser_trace].is_provided())
    {
        std::cout << COLOR_ASCII_BOLD_YELLOW
                << FMT::format(
                    "Flag --{} is disabled, due to OPTIMIZED or HATE_PYTHON build parameters.\n"
                    "Either disable OPTIMIZED_MODE and HATE_PYTHON_MODE or remove flag\n",
                    alpha::flag_show_parser_trace)
                << SGR_RESET << std::endl;
    }
    #endif

    return parser;
}

int main(const int argc, char **argv)
{
    const arguinator::Parser cli_parser = launch_cli_parser(argc, argv);
    alpha::CompilationOptions::Values comp_options = alpha::CompilationOptions::create(cli_parser);

    std::unique_ptr<alpha::Driver> driver;
    try
    {
        const std::string &source_filename = cli_parser[alpha::flag_input_file].get_input();
        driver = std::make_unique<alpha::Driver>(source_filename, comp_options);
        driver->run();
    }
    catch (arguinator::CLIHelp) { return 0; }

    if (cli_parser[alpha::flag_export_symbol_table].is_provided())
        driver->export_symbol_table();
    if (cli_parser[alpha::flag_export_symbol_table_without_temps].is_provided())
        driver->export_symbol_table_without_temps();
    if (cli_parser[alpha::flag_export_diagnostics].is_provided())
        driver->export_diagnostics();
    if (cli_parser[alpha::flag_export_ir].is_provided())
        driver->export_ir();
    if (cli_parser[alpha::flag_show_symbol_table].is_provided())
        driver->show_symbol_table();
    if (cli_parser[alpha::flag_show_ir].is_provided())
        driver->show_ir();
    if (!cli_parser[alpha::flag_no_show_diagnostics].is_provided()) // Used by regression-test tool.
        driver->show_diagnostics();

    return driver->ok() ? 0 : 1;
}
