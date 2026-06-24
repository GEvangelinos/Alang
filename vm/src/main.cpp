#include <filesystem>
#include <fstream>
#include <iostream>

#include "vm/machine.hpp"
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

        const std::string infile_name = cli_parser["source"].get_input();
        std::ifstream infile{infile_name, std::ios::binary};
        if (!infile)
            throw std::runtime_error("Failed opening file for reading: " + infile_name);
        const uintmax_t filesize = std::filesystem::file_size(cli_parser["source"].get_input());
        std::vector<alpha::u8> abc_buffer(filesize);
        infile.read(reinterpret_cast<char*>(abc_buffer.data()), filesize);

        alpha::u32 stack_size = 4096;
        if (cli_parser["stack_size"].is_provided())
            stack_size = std::stoll(cli_parser["stack_size"].get_input());

        const alpha::vm::Executable executable = alpha::ABC_Loader::load(abc_buffer);
        alpha::vm::Machine machine{alpha::vm::Bytes{.count = stack_size}, executable};

        std::ofstream out_file;
        std::ofstream err_file;
        if (cli_parser["out-file"].is_provided())
        {
            out_file = std::ofstream{cli_parser["out-file"].get_input()};
            if (!out_file)
                throw std::runtime_error{"Failed opening VM's out-file"};
            machine.set_out_stream(out_file);
        }
        if (cli_parser["err-file"].is_provided())
        {
            err_file = std::ofstream{cli_parser["err-file"].get_input()};
            if (!err_file)
                throw std::runtime_error{"Failed opening VM's err-file"};
            machine.set_err_stream(err_file);
        }

        machine.run();
    }
    catch (arguinator::CLIHelp) { return 0; }
    catch (arguinator::CLIError& e) { fatal(e); }
    return 0;
}

arguinator::Parser
launch_cli_parser(const int argc, const char* const * const argv)
{
    arguinator::Parser parser(argc, argv, vm_description);
    parser.set_flag("source").set_arity(1).set_help("NYI").set_required();
    parser.set_flag("stack_size").set_arity(1).set_help("NYI");
    parser.set_flag("show_warnings").set_arity(0).set_help("NYI");
    parser.set_flag("out-file").set_arity(1).set_help("NYI");
    parser.set_flag("err-file").set_arity(1).set_help("NYI");

    parser.parse_flags();
    return parser;
}

const char*
fatal_header() { return VM_NAME ": " COLOR_FG_ASCII_BOLD_RED "fatal: " SGR_RESET; }

void
fatal_impl(const std::string_view error_message)
{
    std::cerr << fatal_header() << error_message << std::endl;
    std::exit(EXIT_FAILURE);
}
