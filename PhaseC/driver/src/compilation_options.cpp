#include "driver/compilation_options.hpp"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include "driver/exception.hpp"
#include "arguinator/arguinator.hpp"
#include "support/debug_tools.hpp"
#include "support/format_adapter.hpp"

namespace
{
template<typename T>
[[nodiscard]] T extract(
    const char *flag_name,
    const arguinator::Parser &cli_parser,
    std::function<T(std::string)> extractor,
    const char *type_name)
{
    const std::string &value = cli_parser[flag_name].get_input();
    try { return extractor(value); }
    catch (const std::invalid_argument)
    {
        throw alpha::exception::CLIOptionParseError(flag_name, value, type_name);
    } catch (const std::out_of_range)
    {
        throw alpha::exception::CLIOptionRangeError(flag_name, value, type_name);
    }
    UNREACHABLE("extract(): unexpected fallthrough");
}
} // namespace

namespace alpha
{
const std::vector<OptionSpec> option_specs
{ // clang-format off
    {flag_input_file,            1, true,  "Provide the alpha file to parse."},
    {flag_export_symbol_table,   0, false, "Write the compiler's symbol table to a CSV file named <source_filename>.st.csv."},
    {flag_export_symbol_table_without_temps, 0, false, "Write the compiler's symbol table to a CSV file named <source_filename>.st.csv without the internal temp variables (used for regression testing)."},
    {flag_export_diagnostics,    0, false, "Write the compiler's diagnostics to a CSV file named <source_filename>.diag.csv."},
    {flag_export_ir,             0, false, "Write the compiler's generated quads to <source_filename>.ir.csv."},
    {flag_show_parser_trace,     0, false, "Pretty-prints a message for each matched grammar rule."},
    {flag_show_symbol_table,     0, false, "Pretty-prints the symbol table on console."},
    {flag_show_ir,               0, false, "Pretty-prints the IR quads on console."},
    {flag_list_opts,             0, false, "List available optimizations for IR generation."},
    {flag_max_errors,            1, false, "Set the maximum number of errors before compilation halts."},
    {flag_no_show_diagnostics,   0, false, "Disable diagnostic display (used for regression testing)."},
    {flag_expect_errors,         0, false, "If enable, when errors occur driver still returns EXIT_SUCCESS (used for regression testing error-files)."},
}; // clang-format on

CompilationOptions::Values CompilationOptions::create(const arguinator::Parser &cli_parser)
{
    CompilationOptions::Values cov;
    auto local_stoull = [](const std::string &s) { return std::stoull(s); };
    if (cli_parser[flag_max_errors].is_provided())
        cov.max_errors = extract<std::size_t>(flag_max_errors, cli_parser, local_stoull, "size_t");

    return cov;
}
} // namespace alpha
