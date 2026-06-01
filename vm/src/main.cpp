#include <iostream>

#include "arguinator/arguinator.hpp"
#include "support/cli_color.h"

static constexpr char vm_description[] =
    "A Virtual Machine for running the Alpha language bytecode";
[[nodiscard]] static arguinator::Parser launch_cli_parser(int argc, const char* const * argv);

[[nodiscard]] static const char* fatal_header();
[[noreturn]] static void fatal_impl(std::string_view error_message);
[[noreturn]] static void fatal(const arguinator::CLIError& e) { fatal_impl(e.what()); }

int main(const int argc, char** argv)
{
    try
    {
        const arguinator::Parser cli_parser = launch_cli_parser(argc, argv);

        if (cli_parser["source"].is_provided())
            std::cout << "SOURCE is provided!" << std::endl;


        #error " Load binary file in a buffer as std::vector<u8>"
        // DO NOT THING about direct memory load..

        // We can do this later on.. currently we just need to run.. even with the not so fast way ...
    }
    catch (arguinator::CLIHelp) { return 0; }
    catch (arguinator::CLIError& e) { fatal(e); }
}

arguinator::Parser
launch_cli_parser(const int argc, const char* const * const argv)
{
    arguinator::Parser parser(argc, argv, vm_description);
    parser.set_flag("source").set_arity(1).set_help("NYI").set_required();

    parser.parse_flags();
    return parser;
}

const char*
fatal_header() { return COMPILER_NAME ": " COLOR_FG_ASCII_BOLD_RED "fatal: " SGR_RESET; }

void
fatal_impl(const std::string_view error_message)
{
    std::cerr << fatal_header() << error_message << std::endl;
    std::exit(EXIT_FAILURE);
}
