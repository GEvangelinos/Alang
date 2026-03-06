#ifndef COMPILER_SETTINGS_HPP
#define COMPILER_SETTINGS_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "core/basics.hpp"

// (REQUIRED, ARITY, NAME, HELP_MSG)
#define JOB_SETTINGS(X) \
    X(false, 0, only_show_tokens,    "Pretty-prints all tokens, while reporting lexical errors at end") \
    X(false, 0, list_opts,           "List available optimizations for IR generation.") \
    X(false, 0, show_parser_trace,   "Pretty-prints a message for each matched grammar rule.") \
    X(false, 0, export_symbol_table, "Write the compiler's symbol table to a CSV file named <source_filename>.st.csv.") \
    X(false, 0, show_symbol_table,   "Pretty-prints the symbol table on console.") \
    X(false, 0, export_diagnostics,  "Write the compiler's diagnostics to a CSV file named <source_filename>.diag.csv.") \
    X(false, 0, no_show_diagnostics, "Disable diagnostic display (used for regression testing).")\
    X(false, 0, export_ir,           "Write the compiler's generated quads to <source_filename>.ir.csv.") \
    X(false, 0, show_ir,             "Pretty-prints the IR quads on console.") \
    X(false, 0, show_ir_detailed,    "Pretty-prints the IR quads on console, including liveness metadata (e.g., dead or live quads).") \
    X(false, 0, export_symbol_table_without_temps, "Write the compiler's symbol table to a CSV file named <source_filename>.st.csv without the internal temp variables (used for regression testing).")

// (REQUIRED, ARITY, NAME, HELP_MSG)
#define EXPR_OPT_SETTINGS(X) \
    X(false, 0, opt_const_eval,           "Fold constant expressions in IR (e.g. `x = 1+2;` -> `x = 3;`) and trim redundant operations in IR (e.g. `x = var+0;` -> `x = var;`).")\
    X(false, 0, opt_const_propagation,    "Propagate constant variables in IR (e.g. `x = 5; y = x + 3` -> `y = 5 + 3`).")
// X(false, 0, propagate_const_retvals, "TBI")\
// X(false, 0, remove_dead_branches, "TBI")\
// X(false, 0, remove_unused_vars, "TBI")\
// X(false, 0, remove_unused_funcs, "TBI")\

// (REQUIRED, ARITY, NAME, HELP_MSG)
#define IR_OPT_SETTINGS(X) \
X(false, 0, opt_jump_threading,        "Resolve and fast-forward chains of unconditional jumps (e.g. `goto A; A: goto B;` → `goto B;`) to shorten control-flow paths in the IR.") \
X(false, 0, opt_dead_code_elimination, "Remove IR blocks and branches that are proven unreachable at compile time (e.g. eliminate `if (0)` branches).")

// (REQUIRED, ARITY, NAME, HELP_MSG)
#define CONFIG_FLAG_SETTINGS(X) \
    X(false, 0, expect_errors, "If enable, when errors occur driver still returns EXIT_SUCCESS (used for regression testing error-files).")
// (REQUIRED, ARITY, NAME, VALUE_TYPE, HELP_MSG)
#define CONFIG_DATA_SETTINGS(X) \
    X(true,  1, source,     std::string, "Provide the source file to compile.") \
    X(false, 1, max_errors, std::size_t, "Set the maximum number of errors before compilation halts.")

// Forward Declarations used below:    // clang-format off
namespace arguinator { class Parser; } // clang-format on

namespace alpha
{
struct Setting
{
    bool required;
    std::size_t arity;
    std::string name;
    const char *help;
};

namespace settings
{
    #ifdef REGISTER_SETTING
    #error "Macro collision will occur"
    #endif
    struct Jobs
    {
        #define REGISTER_SETTING(REQUIRED, ARITY, NAME, HELP_MSG) \
            static constexpr char NAME[] = #NAME;
        JOB_SETTINGS(REGISTER_SETTING)
        #undef  REGISTER_SETTING
    };

    struct ExprOpts
    {
        #define REGISTER_SETTING(REQUIRED, ARITY, NAME, HELP) bool NAME = false;
        EXPR_OPT_SETTINGS(REGISTER_SETTING)
        #undef  REGISTER_SETTING
    };

    struct IROpts
    {
        #define REGISTER_SETTING(REQUIRED, ARITY, NAME, HELP) bool NAME = false;
        IR_OPT_SETTINGS(REGISTER_SETTING)
        #undef  REGISTER_SETTING
    };

    struct ConfigFlags
    {
        #define REGISTER_SETTING(REQUIRED, ARITY, NAME, HELP) bool NAME = false;
        CONFIG_FLAG_SETTINGS(REGISTER_SETTING)
        #undef  REGISTER_SETTING
    };

    struct ConfigData
    {
        #define REGISTER_SETTING(REQUIRED, ARITY, NAME, VALUE_TYPE, HELP) VALUE_TYPE NAME;
        CONFIG_DATA_SETTINGS(REGISTER_SETTING)
        #undef  REGISTER_SETTING
    };
} // namespace Settings

class SettingManager
{
public:
    SettingManager();

    void parse_settings(const arguinator::Parser &arg_parser);

    [[nodiscard]] const auto &all_settings() const noexcept { return all_settings_; }
    [[nodiscard]] const settings::ExprOpts &expr_opt_settings() const;
    [[nodiscard]] const settings::IROpts &ir_opt_settings() const;
    [[nodiscard]] const settings::ConfigFlags &config_flag_settings() const;
    [[nodiscard]] const settings::ConfigData &config_data_settings() const;

private:
    const std::vector<Setting> all_settings_;
    Once<settings::ConfigFlags> config_flag_settings_;
    Once<settings::ConfigData> config_data_settings_;
    Once<settings::ExprOpts> expr_opt_settings_;
    Once<settings::IROpts> ir_opt_settings_;

    [[nodiscard]] static settings::ConfigFlags parse_config_flag_settings(
        const arguinator::Parser &arg_parser);
    [[nodiscard]] static settings::ConfigData parse_config_data_settings(
        const arguinator::Parser &arg_parser);
    [[nodiscard]] static settings::ExprOpts parse_expr_opt_settings(
        const arguinator::Parser &arg_parser);
    [[nodiscard]] static settings::IROpts parse_ir_opt_settings(
        const arguinator::Parser &arg_parser);
};
} // namespace alpha

#endif // COMPILER_SETTINGS_HPP
