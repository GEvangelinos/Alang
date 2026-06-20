#include <set>
#include <string>

#include "arguinator/arguinator.hpp"
#include "driver/alpha_driver.hpp"

#include "driver/exception.hpp"
#include "settings/compiler_settings.hpp"
#include "support/cli_color.h"

bool g_show_parser_trace = false;

static constexpr char compiler_description[] = "A Compiler for the Alpha language";

[[nodiscard]] static arguinator::Parser launch_cli_parser(
    int argc, const char* const * argv, const alpha::SettingManager& sm);

static void handle_exports(const arguinator::Parser& cli_parser, const alpha::Driver& driver);
static void handle_shows(const arguinator::Parser& cli_parser, const alpha::Driver& driver);
[[nodiscard]] static const char* fatal_header();
[[noreturn]] static void fatal_impl(std::string_view error_message);
[[noreturn]] static void fatal(const alpha::exception::DriverError& e) { fatal_impl(e.what()); }
[[noreturn]] static void fatal(const arguinator::CLIError& e) { fatal_impl(e.what()); }

int main(const int argc, char** argv)
{
    alpha::SettingManager setting_manager;
    std::unique_ptr<alpha::Driver> driver;
    try
    {
        const arguinator::Parser cli_parser = launch_cli_parser(argc, argv, setting_manager);
        setting_manager.parse_settings(cli_parser);

        driver = std::make_unique<alpha::Driver>(setting_manager);

        if (cli_parser[alpha::settings::Jobs::only_show_tokens].is_provided())
            driver->only_lex_tokens();
        else
        {
            driver->run();
            handle_exports(cli_parser, *driver);
            handle_shows(cli_parser, *driver);
        }
    }
    catch (arguinator::CLIHelp) { return 0; }
    catch (arguinator::CLIError& e) { fatal(e); }
    catch (alpha::exception::DriverError& e) { fatal(e); }

    if (setting_manager.config_flag_settings().expect_errors)
        return driver->ok() ? EXIT_FAILURE : EXIT_SUCCESS;
    return driver->ok() ? EXIT_SUCCESS : EXIT_FAILURE;
}

arguinator::Parser
launch_cli_parser(const int argc, const char* const * const argv, const alpha::SettingManager& sm)
{
    arguinator::Parser parser(argc, argv, compiler_description);

    for (const auto& s : sm.all_settings())
    {
        auto& flag = parser.set_flag(s.name)
                           .set_arity(s.arity)
                           .set_help(s.help);
        if (s.required)
            flag.set_required();
    }

    parser.parse_flags();

#if defined(OPTIMIZED_MODE) || defined(HATE_PYTHON_MODE)
    if (parser[alpha::settings::Jobs::show_parser_trace].is_provided())
    {
        std::cout << COLOR_FG_ASCII_BOLD_YELLOW
            << FMT::format(
                "Flag --{} is disabled, due to OPTIMIZED or HATE_PYTHON build parameters.\n"
                "Either disable OPTIMIZED_MODE and HATE_PYTHON_MODE or remove flag\n",
                alpha::settings::Jobs::show_parser_trace)
            << SGR_RESET << std::endl;
    }
#endif
    return parser;
}

void handle_exports(const arguinator::Parser& cli_parser, const alpha::Driver& driver)
{
    using ASJ = alpha::settings::Jobs;
    if (cli_parser[ASJ::export_symbol_table].is_provided())
        driver.export_symbol_table();
    if (cli_parser[ASJ::export_symbol_table_without_temps].is_provided())
        driver.export_symbol_table_without_temps();
    if (cli_parser[ASJ::export_diagnostics].is_provided())
        driver.export_diagnostics();
    if (cli_parser[ASJ::export_ir].is_provided())
        driver.export_ir();
    if (!cli_parser[ASJ::no_bin].is_provided())
        driver.emit_abc();
}

void handle_shows(const arguinator::Parser& cli_parser, const alpha::Driver& driver)
{
    using ASJ = alpha::settings::Jobs;
    if (cli_parser[ASJ::show_symbol_table].is_provided())
        driver.show_symbol_table();
    if (cli_parser[ASJ::show_symbol_table_without_temps].is_provided())
        driver.show_symbol_table_without_temps();
    if (cli_parser[ASJ::show_ir].is_provided())
        driver.show_ir(false);
    if (cli_parser[ASJ::show_ir_detailed].is_provided())
        driver.show_ir(true);
    if (cli_parser[ASJ::show_abc].is_provided())
        driver.show_abc();
    if (!cli_parser[ASJ::no_show_diagnostics].is_provided())
        driver.show_diagnostics(); // Used by regression-test tool.
}

const char*
fatal_header() { return COMPILER_NAME ": " COLOR_FG_ASCII_BOLD_RED "fatal: " SGR_RESET; }

void fatal_impl(const std::string_view error_message)
{
    std::cerr << fatal_header() << error_message << std::endl;
    std::exit(EXIT_FAILURE);
}