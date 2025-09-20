#include <string>
#include <driver/compilation_options.hpp>
#include "arguinator/arguinator.hpp"
#include "driver/alpha_driver.hpp"

#include "driver/exception.hpp"
#include "support/cli_color.h"

#if defined(OPTIMIZED_MODE) || defined(HATE_PYTHON_MODE)
#include "support/cli_color.h"
#endif

static constexpr char compiler_description[] = "A Compiler for the Alpha language";

[[nodiscard]] static arguinator::Parser launch_cli_parser(int argc, const char *const *argv);
void handle_exports(const arguinator::Parser &cli_parser, const alpha::Driver &driver);
void handle_shows(const arguinator::Parser &cli_parser, const alpha::Driver &driver);
[[nodiscard]] static const char *fatal_header();
[[noreturn]] static void fatal_impl(std::string_view error_message);
[[noreturn]] static void fatal(const alpha::exception::DriverError &e) { fatal_impl(e.what()); }
[[noreturn]] static void fatal(const arguinator::CLIError &e) { fatal_impl(e.what()); }

int main(const int argc, char **argv)
{
    bool expect_errors = false;
    std::unique_ptr<alpha::Driver> driver;
    try
    {
        const arguinator::Parser cli_parser = launch_cli_parser(argc, argv);

        expect_errors = cli_parser[alpha::flag_expect_errors].is_provided();

        namespace aoc = alpha::CompilationOptions;
        aoc::Values comp_options = aoc::create(cli_parser);
        const std::string &source_filename = cli_parser[alpha::flag_input_file].get_input();

        driver = std::make_unique<alpha::Driver>(source_filename, comp_options);
        driver->run();
        handle_exports(cli_parser, *driver);
        handle_shows(cli_parser, *driver);
    }
    catch (arguinator::CLIHelp) { return 0; }
    catch (arguinator::CLIError &e) { fatal(e); }
    catch (alpha::exception::DriverError &e) { fatal(e); }

    if (expect_errors)
        return driver->ok() ? EXIT_FAILURE : EXIT_SUCCESS;
    return driver->ok() ? EXIT_SUCCESS : EXIT_FAILURE;
}

arguinator::Parser
launch_cli_parser(const int argc, const char *const *const argv)
{
    arguinator::Parser parser(argc, argv, compiler_description);

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

void handle_exports(const arguinator::Parser &cli_parser, const alpha::Driver &driver)
{
    if (cli_parser[alpha::flag_export_symbol_table].is_provided())
        driver.export_symbol_table();
    if (cli_parser[alpha::flag_export_symbol_table_without_temps].is_provided())
        driver.export_symbol_table_without_temps();
    if (cli_parser[alpha::flag_export_diagnostics].is_provided())
        driver.export_diagnostics();
    if (cli_parser[alpha::flag_export_ir].is_provided())
        driver.export_ir();
}

void handle_shows(const arguinator::Parser &cli_parser, const alpha::Driver &driver)
{
    if (cli_parser[alpha::flag_show_symbol_table].is_provided())
        driver.show_symbol_table();
    if (cli_parser[alpha::flag_show_ir].is_provided())
        driver.show_ir();
    if (!cli_parser[alpha::flag_no_show_diagnostics].is_provided())
        driver.show_diagnostics(); // Used by regression-test tool.
}

const char *
fatal_header() { return COMPILER_NAME ": " COLOR_ASCII_BOLD_RED "fatal: " SGR_RESET; }

void
fatal_impl(const std::string_view error_message)
{
    std::cerr << fatal_header() << error_message << std::endl;
    std::exit(EXIT_FAILURE);
}
