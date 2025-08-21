#ifndef COMPILATION_OPTIONS_HPP
#define COMPILATION_OPTIONS_HPP
#include <limits>
#include <vector>

// Forward Declarations used below:    // clang-format off
namespace arguinator { class Parser; } // clang-format on

namespace alpha
{
struct OptionSpec
{
    const char *name;
    std::size_t arity;
    bool required;
    const char *help;
};

extern const std::vector<OptionSpec> option_specs;

static constexpr char flag_input_file[] = "input-file";
static constexpr char flag_export_symbol_table[] = "export-symbol-table";
static constexpr char flag_export_compile_errors[] = "export-compile-errors";
static constexpr char flag_export_quads[] = "export-quads";
static constexpr char flag_show_symbol_table[] = "show-symbol-table";
static constexpr char flag_show_parser_trace[] = "show-parser-trace";
static constexpr char flag_no_show_errors[] = "no-show-errors";
static constexpr char flag_show_quads[] = "show-quads";
static constexpr char flag_list_opts[] = "list-opts";
static constexpr char flag_max_errors[] = "max_errors";

namespace CompilationOptions
{
    struct Values
    {
        std::size_t max_errors = std::numeric_limits<std::size_t>::max();
    };

    Values create(const arguinator::Parser &cli_parser);
} // namespace CompilationOptions
} // namespace alpha
#endif // COMPILATION_OPTIONS_HPP
