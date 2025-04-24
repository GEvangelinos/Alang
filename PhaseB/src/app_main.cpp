#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include "utils/format_adapter.hpp"
#include "alpha_driver.hpp"
#include "arguinator/arguinator.hpp"
#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_location.hpp"
#include "scanner/alpha_scanner_context.hpp"
#include "alpha_scanner.hpp"
#include "core/alpha_konstants.hpp"
#include "alpha_parser.hpp"
#include "core/alpha_error_tracker.hpp"
#include "core/alpha_types.hpp"
#include "utils/cli_color.h"

bool g_show_parser_trace = false;

static constexpr char alpha_driver_description[] =
		"A tool for syntactical analysis on programming language Alpha";

static constexpr char flag_input_file[] = "input-file";
static constexpr char flag_export_symtable[] = "export-symbol-table";
static constexpr char flag_export_symtable_path[] = "symbol-table-export-path";
static constexpr char flag_show_symbol_table[] = "show-symbol-table";
static constexpr char flag_show_parser_trace[] = "show-parser-trace";

static Arguinator::Parser launch_cli_parser(int argc, const char *const *const argv)
{
	Arguinator::Parser parser(argc, argv, alpha_driver_description);

	parser.set_flag(flag_input_file)
			.set_arity(1)
			.set_required()
			.set_help("Use flag to provide the alpha file you want to parse.");

	parser.set_flag(flag_export_symtable)
			.set_arity(0)
			.set_help("If set, write the compiler's symbol table to a CSV file named "
								"<source_filename>.csv for external inspection.");

	parser.set_flag(flag_export_symtable_path)
			.set_arity(1)
			.set_help("Specifies the output path (file or directory) for the symbol table CSV. "
								"If a directory is given, the file will be named '<source_filename>.symtable.csv'.");

	parser.set_flag(flag_show_symbol_table)
			.set_arity(0)
			.set_help("Pretty-prints the symbol table on console");

	// In optimized mode showing parser trace in not available
	parser.set_flag(flag_show_parser_trace)
			.set_arity(0)
			.set_help("Pretty-prints a string message for each matched rule on parser's grammar");

	parser.parse_flags();

	return parser; // NRVO
}

int main(int argc, char **argv)
{
	const Arguinator::Parser cli_parser = launch_cli_parser(argc, argv);
	Alpha::Driver driver(cli_parser[flag_input_file].get_input());
	g_show_parser_trace = cli_parser[flag_show_parser_trace].is_provided();
	driver.run_syntax_analyzer();

	if (cli_parser[flag_export_symtable].is_provided())
	{
		if (cli_parser[flag_export_symtable_path].is_provided())
			driver.export_symbol_table(cli_parser[flag_export_symtable_path].get_input());
		else
			driver.export_symbol_table(std::nullopt);
	}

	if (cli_parser[flag_show_symbol_table].is_provided())
		driver.display_symbol_table();

	driver.display_compilation_errors();

	return driver.ok() ? 0 : 1;
}
