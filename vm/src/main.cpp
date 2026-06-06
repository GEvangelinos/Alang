#include <filesystem>
#include <fstream>
#include <iostream>

#include "../include/vm/machine.hpp"
#include "arguinator/arguinator.hpp"
#include "support/cli_color.h"
#include "core/numeric_types.hpp"
#include "bytecode/abc_loader.hpp"

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

        const std::string infile_name = cli_parser["source"].get_input();
        std::ifstream infile{infile_name, std::ios::binary};
        if (!infile)
            throw std::runtime_error("Failed opening file for reading: " + infile_name);
        const uintmax_t filesize = std::filesystem::file_size(cli_parser["source"].get_input());
        std::vector<alpha::u8> abc_buffer(filesize);
        infile.read(reinterpret_cast<char*>( abc_buffer.data()), filesize);
        const alpha::vm::Executable executable = alpha::ABC_Loader::load(abc_buffer);


        alpha::vm::Machine machine{alpha::vm::Bytes::from_KB(128)};
        machine.run();
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
