#include "driver/compilation_options.hpp"

#include <stdexcept>
#include <string>
#include "arguinator/arguinator.hpp"

namespace alpha
{
const std::vector<OptionSpec> option_specs
{ // clang-format off
        {flag_input_file,            1, true,  "Provide the alpha file to parse."},
        {flag_export_symbol_table,   0, false, "Write the compiler's symbol table to a CSV file named <source_filename>.st.csv."},
        {flag_export_compile_errors, 0, false, "Write the compiler's errors to a CSV file named <source_filename>.error.csv."},
        {flag_export_quads,          0, false, "Write the compiler's generated quads to <source_filename>.quads."},
        {flag_show_parser_trace,     0, false, "Pretty-prints a message for each matched grammar rule."},
        {flag_show_symbol_table,     0, false, "Pretty-prints the symbol table on console."},
        {flag_show_quads,            0, false, "Pretty-prints the IR quads on console."},
        {flag_no_show_errors,        0, false, "Disable error display (used for regression testing)."},
        {flag_list_opts,             0, false, "List available optimizations for IR generation."},
        {flag_max_errors,            1, false, "Set the maximum number of errors before compilation halts."},
}; // clang-format on

CompilationOptions::Values create(const arguinator::Parser &cli_parser)
{
    CompilationOptions::Values cov;

    try
    {
        if (cli_parser[flag_max_errors].is_provided())
            cov.max_errors = std::stoull(cli_parser[flag_max_errors].get_input());
    }
    catch (std::invalid_argument &e) {}
    catch (std::out_of_range &e) {}

    return std::move(cov);
}
} // namespace alpha
