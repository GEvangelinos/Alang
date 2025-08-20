#include <string>                    // for basic_string, string
#include "arguinator/arguinator.hpp" // for Flag, Parser
#include "driver/alpha_driver.hpp"
#include <driver/compilation_options.hpp>
#include "utils/cli_color.h"

static constexpr char alpha_driver_description[] =
        "A tool for syntactical analysis on programming language Alpha";

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
    if (parser[flag_show_parser_trace].is_provided())
    {
        std::cout << COLOR_ASCII_BOLD_YELLOW
                << FMT::format(
                    "Flag --{} is disabled, due to OPTIMIZED or HATE_PYTHON build parameters.\n"
                    "Either disable OPTIMIZED_MODE and HATE_PYTHON_MODE or remove flag\n",
                    flag_show_parser_trace)
                << SGR_RESET << std::endl;
    }
    #endif

    return std::move(parser);
}

int main(const int argc, char **argv)
{
    const arguinator::Parser cli_parser = launch_cli_parser(argc, argv);

    std::unique_ptr<alpha::Driver> driver;
    try
    {
        const std::string &source_filename = cli_parser[alpha::flag_input_file].get_input();
        driver = std::make_unique<alpha::Driver>(source_filename, );
    }
    catch (arguinator::CLIHelp) { return 0; }

    driver->run();

    if (cli_parser[alpha::flag_export_symbol_table].is_provided())
        driver->export_symbol_table();
    if (cli_parser[alpha::flag_export_compile_errors].is_provided())
        driver->export_compile_errors();
    if (cli_parser[alpha::flag_export_quads].is_provided())
        driver->export_quads();
    if (cli_parser[alpha::flag_show_symbol_table].is_provided())
        driver->show_symbol_table();
    if (cli_parser[alpha::flag_show_quads].is_provided())
        driver->show_quads();
    if (!cli_parser[alpha::flag_no_show_errors].is_provided()) // Used by regression-test tool.
        driver->show_compile_issues();

    return driver->ok() ? 0 : 1;
}
