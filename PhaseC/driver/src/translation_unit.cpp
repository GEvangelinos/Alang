#include <alpha_parser.gen.hpp>
#include <fstream>
#include <driver/translation_unit.hpp>

#include "ir_optimizer/ir_optimizer.hpp"
#include "core/konstants.hpp"
#include "driver/exception.hpp"
#include "driver/konstants.hpp"
#include "core/exception.hpp"
#include "core/translation_unit_buffer.hpp"
#include "driver/translation_unit_buffer_loader.hpp"
#include "scanner/alpha_scanner.gen.hpp"
#include "scanner/scanner_adapter.hpp"

#include "support/cli_color.h"

#define WARNING_BANNER_PREFIX COLOR_FG_ASCII_MAGENTA SGR_BOLD "Warning" SGR_BLINK "❗" SGR_RESET ": "
inline constexpr auto k_reescaped_warning_banner =
    WARNING_BANNER_PREFIX
    "Special characters are escaped; Ex.: \"\\n\" prints literally (not a newline) to keep the table aligned.\n"
    SGR_RESET;

// Note: The following stirng contains Greek characters.
// I dont remember what's going on with unicode and C++
// but in case it causes any problems just remove the Greek part
// It is just a reference to a lecture.
inline constexpr auto k_temp_reuse_warning_banner =
    WARNING_BANNER_PREFIX
    "Temporaries are context-local. Each function starts with a fresh temp frame (counter reset to zero).\n"
    "           Nested function definitions or functions assigned to tables/variables therefore allocate temps from a\n"
    "           clean slate. When the function definition ends, the previous temp counter is restored. (Lecture 9 Slide 40)\n"
    "           >>> Οι κρυφές μεταβλητές είναι κανονικές μεταβλητές και η δημιουργία\n"
    "           >>> τους απαιτεί δημιουργία νέου συμβόλου στον πίνακα συμβόλων, ενώ\n"
    "           >>> απενεργοποιούνται κανονικά εκτός της εμβέλειας δήλωσής τους\n"
    SGR_RESET;

inline constexpr auto k_cya_mode_off_warning_banner =
    WARNING_BANNER_PREFIX
    "This executable was built with CYA_MODE disabled. As a result:\n"
    "           a) Aggressive reuse of temporary variables is enabled; table literals are handled differently.\n"
    "           b) Assignment temps are no longer created except when strictly required (e.g., inside function calls).\n"
    SGR_RESET;

namespace
{
void create_export_directory(std::string_view dirname);
void enter_export_directory(std::string_view dirname);
void exit_export_directory(const std::filesystem::path& original_path);
std::string expr_printer(const alpha::Expr* expr, const char* missing_marker = "");

template <unsigned column, unsigned column_width, typename T>
std::string color_column(T&& value);
template <bool colorize, unsigned column, unsigned column_width, typename T>
std::string format_column(T&& value);
template <bool colorize, typename Stream>
void print_ir(
    Stream& out,
    const std::vector<alpha::Quad>& quads,
    const alpha::LocationTracker& lt,
    bool print_detailed);

void create_export_directory(const std::string_view dirname)
{
    std::filesystem::create_directories(dirname);
}

void enter_export_directory(std::string_view dirname) { std::filesystem::current_path(dirname); }

void exit_export_directory(const std::filesystem::path& original_path)
{
    std::filesystem::current_path(original_path);
}

std::string escape(const char* const str)
{
    std::string out;
    char ch;
    for (std::size_t i = 0; (ch = str[i]) != '\0'; ++i)
    {
        switch (ch)
        { // clang-format off
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        case '\v': out += "\\v"; break;
        case '\f': out += "\\f"; break;
        case '\b': out += "\\b"; break;
        case '\\': out += "\\\\"; break;
        case '\"': out += "\\\""; break;
        default: out += ch; break;
        } // clang-format on
    }
    return out;
}

std::string expr_printer(const alpha::Expr* expr, const char* const missing_marker)
{
    using namespace alpha;
    if (!expr)
        return missing_marker;
    using ET = Expr::Type;
    switch (expr->type)
    {
    case ET::CONST_BOOL: return static_cast<const ConstBoolExpr*>(expr)->value ? "true" : "false";
    case ET::CONST_INT: return std::to_string(static_cast<const ConstIntExpr*>(expr)->value);
    case ET::CONST_FLOAT: return std::to_string(static_cast<const ConstFloatExpr*>(expr)->value);
    case ET::CONST_STRING:
        return FMT::format("\"{}\"", escape(static_cast<const ConstStringExpr*>(expr)->value));
    case ET::CONST_NIL: return "nil";
    case ET::ARITHMETIC: return static_cast<const ArithmeticExpr*>(expr)->var_symbol->name;
    case ET::ASSIGN: return static_cast<const AssignExpr*>(expr)->var_symbol->name;
    case ET::BOOL: return static_cast<const BoolExpr*>(expr)->var_symbol->name;
    case ET::LIBRARY_FUNCTION: return static_cast<const LibFuncExpr*>(expr)->libfunc_symbol->name;
    case ET::NEW_TABLE: return static_cast<const NewTableExpr*>(expr)->var_symbol->name;
    case ET::PROGRAM_FUNCTION: return static_cast<const ProgFuncExpr*>(expr)->progfunc_symbol->
            name;
    case ET::TABLE_ITEM: return static_cast<const TableItemExpr*>(expr)->var_symbol->name;
    case ET::VARIABLE: return static_cast<const VariableExpr*>(expr)->var_symbol->name;
    default:
        UNREACHABLE(FMT::format("Unhandled Expr::Type: int({}) = {}",
            TO_STRING(expr->type), static_cast<int>(expr->type)
        ));
    }
}

template <unsigned Column, unsigned ColumnWidth, typename T>
std::string color_column(T&& value)
{
    constexpr unsigned column_count = 8;
    static_assert(Column < column_count, "So far we support a maximum of 8 columns");

    const char* ascii_color;
    if constexpr (Column == 0) ascii_color = COLOR_FG_ASCII_WHITE;
    else if constexpr (Column == 1) ascii_color = COLOR_FG_ASCII_RED;
    else if constexpr (Column == 2) ascii_color = COLOR_FG_ASCII_GREEN;
    else if constexpr (Column == 3) ascii_color = COLOR_FG_ASCII_BLUE;
    else if constexpr (Column == 4) ascii_color = COLOR_FG_ASCII_CYAN;
    else if constexpr (Column == 5) ascii_color = COLOR_FG_ASCII_YELLOW;
    else if constexpr (Column == 6) ascii_color = COLOR_FG_ASCII_MAGENTA;
    else if constexpr (Column == 7) ascii_color = COLOR_FG_MATRIX;
    else ascii_color = COLOR_FG_ASCII_DEFAULT;
    return FMT::format("{}{:<{}}{}", ascii_color, std::forward<T>(value), ColumnWidth, SGR_RESET);
}

template <bool Colorize, unsigned Column, unsigned ColumnWidth, typename T>
std::string format_column(T&& value)
{
    if constexpr (Colorize)
        return color_column<Column, ColumnWidth>(std::forward<T>(value));
    else
        return FMT::format("{:<{}}", std::forward<T>(value), ColumnWidth);
}

template <bool Colorize, typename Stream>
void print_ir(
    Stream& out,
    const std::vector<alpha::Quad>& quads,
    const alpha::LocationTracker& lt,
    const bool print_detailed)
{
    constexpr alpha::u32 widths[] = {10, 15, 20, 20, 20, 10, 10, 10};
    constexpr alpha::u32 quad_header_width = [&widths]() constexpr
    {
        alpha::u32 width = 0;
        for (alpha::u32 i = 0; i < std::size(widths); ++i)
            width += widths[i];
        width += std::size(widths) - 1; // One space between each column
        return width;
    }();

    out << k_reescaped_warning_banner;
    out << k_temp_reuse_warning_banner;
    out << k_cya_mode_off_warning_banner;

    // Write export header.
    out << FMT::format(
        "{0} {1} {2} {3} {4} {5} {6} {7}\n",
        format_column<Colorize, 0, widths[0]>("quad#"),
        format_column<Colorize, 1, widths[1]>("opcode"),
        format_column<Colorize, 2, widths[2]>("result"),
        format_column<Colorize, 3, widths[3]>("arg1"),
        format_column<Colorize, 4, widths[4]>("arg2"),
        format_column<Colorize, 5, widths[5]>("label"),
        format_column<Colorize, 6, widths[6]>("line"),
        format_column<Colorize, 7, widths[7]>(print_detailed ? "LIVENESS" : "")
    );

    // Write separating dash line.
    out << std::string(quad_header_width, '-') << '\n';

    // Write quads.
    const auto quads_size = quads.size();
    for (alpha::u32 i = 0; i < quads_size; i++)
    {
        const alpha::Quad& q = quads[i];

        std::string quad_label_str = alpha::ir::info_traits::is_branching(quads[i].opcode)
                                     ? std::to_string(q.label)
                                     : alpha::k_not_available_pretty_marker;

        const auto [first_line, last_line] = lt.find_lines(q.loc);
        std::string quad_line_str = first_line == last_line
                                    ? std::to_string(first_line.value)
                                    : FMT::format("{}-{}", first_line.value, last_line.value);

        out << FMT::format(
            "{0} {1} {2} {3} {4} {5} {6} {7}\n",
            format_column<Colorize, 0, widths[0]>(i + 1), // +1 as, 0 is indicating no-address
            format_column<Colorize, 1, widths[1]>(to_string(q.opcode)),
            format_column<Colorize, 2, widths[2]>(expr_printer(q.result)),
            format_column<Colorize, 3, widths[3]>(expr_printer(q.arg1)),
            format_column<Colorize, 4, widths[4]>(expr_printer(q.arg2)),
            format_column<Colorize, 5, widths[5]>(quad_label_str),
            format_column<Colorize, 6, widths[6]>(quad_line_str),
            format_column<Colorize, 7, widths[7]>(print_detailed && q.is_dead ? "DEAD" : "")
        );
    }
    if constexpr (Colorize)
        out << SGR_RESET;
    out << std::endl;
}
} // namespace
namespace alpha
{
inline constexpr auto k_scanner_eof_null_padding = 2; // For 2 consecutive NULL bytes.

PassManager::PassManager(
    const alpha::settings::ExprOpts& expr_opts,
    const alpha::settings::IROpts& ir_opts,
    TranslationUnitBuffer& tu_buffer,
    LocationTracker& lt,
    DiagnosticEngine& diagnostic_engine,
    SymbolTable* const symbol_table)
    : lt_(lt),
      diagnostic_engine_(diagnostic_engine),
      parse_ctx_(support::require_ptr(symbol_table)),
      lexer_ctx_(),
      scanner_(std::make_unique<ScannerAdapter>(
          lexer_ctx_, lt, *support::require_ptr(diagnostic_engine.reporter.get()), tu_buffer
      )),
      semantic_system_(
          expr_opts,
          &parse_ctx_,
          support::require_ptr(symbol_table),
          diagnostic_engine_.reporter.get()
      ),
      ir_optimizer_(std::make_unique<IROptimizer>(ir_opts)) { DEBUG_SMART_ASSERT(!!ir_optimizer_); }

void
PassManager::execute()
{
    run_frontend();
    ir_quads_ = semantic_system_.gateway->extract_quads();
    ir_quads_ = ir_optimizer_->run(std::move(ir_quads_));
}

void
PassManager::run_frontend()
{
    // The following are the possible return values yyparse can return (based on bison's manual).
    // constexpr auto successful_parsing = 0;
    // constexpr auto invalid_input = 1;
    // constexpr auto memory_exhaustion = 2;

    running_phase_ = Phase::FRONTEND;
    parser_retval_ = alpha_yyparse(
        *scanner_,
        lexer_ctx_,
        lt_,
        diagnostic_engine_,
        *DEBUG_REQUIRE_PTR(diagnostic_engine_.reporter.get()),
        semantic_system_
    );
}

bool
PassManager::is_in_hard_error() const
{
    switch (running_phase_)
    {
    case Phase::FRONTEND: return semantic_system_.good();
    case Phase::IR_OPTIMIZATION:
        DEBUG_SMART_ASSERT(false && "No such query should happen at this state (logically)");
        return false;
    default: UNREACHABLE("Unknown `Phase`");
    }
}

const std::vector<Quad>&
PassManager::get_quads() const noexcept { return ir_quads_; }

void
PassManager::notify_hard_error()
{
    switch (running_phase_)
    {
    case Phase::FRONTEND: return semantic_system_.gateway->notify_hard_error();
    default: UNREACHABLE("Unknown `running_phase`");
    }
}

DiagnosticEngine::Policy
TranslationUnit::create_diagnostic_engine_policy()
{
    return DiagnosticEngine::Policy{
        .should_emit_diagnostic = [this]() { return pass_manager_->is_in_hard_error(); },
        .notify_max_errors_reached = []() { notify_max_errors_reached(); },
        .notify_fatal_error = [this]()
        {
            pass_manager_->notify_hard_error();
            notify_fatal_error();
        },
        .notify_hard_error = [this]() { pass_manager_->notify_hard_error(); }
    };
}

TranslationUnit::TranslationUnit(
    const std::filesystem::path& source_path,
    const std::size_t max_errors,
    const settings::ExprOpts& expr_opts,
    const settings::IROpts& ir_opts)
    : source_path_(source_path),
      expr_opts_(expr_opts),
      diagnostic_engine_(create_diagnostic_engine_policy(), max_errors),
      translation_unit_buffer_(
          TranslationUnitBufferLoader::load_tub(source_path, k_scanner_eof_null_padding)
      ),
      loc_tracker_((translation_unit_buffer_->size() - translation_unit_buffer_->null_padding).value),
      diagnostic_formatter_(
          source_path, loc_tracker_, *support::require_ptr(translation_unit_buffer_.get()), true
      ),
      symbol_table_(),
      pass_manager_(std::make_unique<PassManager>(
          expr_opts,
          ir_opts,
          *support::require_ptr(translation_unit_buffer_.get()),
          loc_tracker_,
          diagnostic_engine_,
          &symbol_table_
      )) {}

TranslationUnit::~TranslationUnit() = default;


void
TranslationUnit::compile()
{
    struct FreezeOnExit // A way to run code before function frame collapses (return, throw)
    {
        LocationTracker& loc_tracker;
        explicit FreezeOnExit(LocationTracker& loc_tracker) : loc_tracker(loc_tracker) {}
        ~FreezeOnExit() { loc_tracker.lines_frozen.raise(); }
    } freeze_on_exit(loc_tracker_);

    tried_compiling.raise();
    try
    {
        pass_manager_->execute();
        // If we reach this point, execute() completed without throwing any exceptions.
        execution_completed_ = true;
    }
    catch (exception::SanityLimitError& e)
    {
        std::cerr << FMT::format("Sanity limit exceeded: {}", e.what()) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (exception::DiagnosticFatalError) { /* Reached a fatal diagnostic - stop compilation. */ }
    catch (exception::DiagnosticLimitError) { /* Reached diagnostics limit  - stop compilation. */ }
}

void
TranslationUnit::notify_fatal_error() { throw exception::DiagnosticFatalError(); }

void
TranslationUnit::notify_max_errors_reached() { throw exception::DiagnosticLimitError(); }

void
TranslationUnit::show_symbol_table() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    std::cout << COLOR_FG_ASCII_BLUE;
    const auto& symbol_per_scope_vector = symbol_table_.symbols_per_scope();
    for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
    {
        if (symbol_per_scope_vector[scope].empty())
            continue;
        std::cout << FMT::format(
            "----------------------------     Scope #{:<4}     ----------------------------\n",
            scope
        );
        for (const auto symbol_ptr : symbol_per_scope_vector[scope])
            std::cout << FMT::format("{:<30} {:<20} (line {:>5}) (scope {:>4})\n",
                                     FMT::format("\"{}\"", symbol_ptr->name),
                                     FMT::format("[{}]", symbol_ptr->type_to_string()),
                                     loc_tracker_.find_symbol_line(symbol_ptr->loc).value,
                                     symbol_ptr->scope);
        std::cout << '\n';
    }
    std::cout << SGR_RESET << std::endl;
}

void
TranslationUnit::show_diagnostics() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    const std::string source_filename = source_path_.filename().string();

    for (const auto& diagnostic : diagnostic_engine_.get_diagnostics())
        std::cerr << diagnostic_formatter_.format(*diagnostic);
    std::cerr << std::endl;
}

void
TranslationUnit::show_ir(const bool detailed) const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    print_ir<true>(std::cout, pass_manager_->get_quads(), loc_tracker_, detailed);
}

void
TranslationUnit::export_symbol_table() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    export_within_dir(k_symbol_table_exports_dirname,
                      &TranslationUnit::export_symbol_table_dispatch);
}

void
TranslationUnit::export_symbol_table_without_temps() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    export_within_dir(k_symbol_table_exports_dirname,
                      &TranslationUnit::export_symbol_table_without_temps_dispatch);
}

void
TranslationUnit::export_diagnostics() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    export_within_dir(k_diagnostic_exports_dirname,
                      &TranslationUnit::export_diagnostics_impl);
}

void
TranslationUnit::export_ir() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    export_within_dir(k_ir_exports_dirname, &TranslationUnit::export_ir_impl);
}

bool TranslationUnit::compiled_ok() const noexcept
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    return execution_completed_ && !diagnostic_engine_.has_errors();
}

void
TranslationUnit::export_symbol_table_dispatch() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    export_symbol_table_impl(true);
}

void
TranslationUnit::export_symbol_table_without_temps_dispatch() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    export_symbol_table_impl(false);
}

void
TranslationUnit::export_symbol_table_impl(const bool export_temps) const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    const std::string outfile_name = source_path_.filename().string() + k_symbol_table_export_ext;
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(
            FMT::format("Failed opening file {} to export symbol table", outfile_name));

    outfile << k_symbol_table_csv_export_header; // Write CSV header.

    auto write_symbol_line = [&](const Symbol* symbol_ptr)
    {
        outfile << FMT::format(
            "{0},{1},{2},{3}\n",
            symbol_ptr->name,
            symbol_ptr->type_to_string(),
            loc_tracker_.find_symbol_line(symbol_ptr->loc).value,
            symbol_ptr->scope
        );
    };

    const auto& symbol_per_scope_vector = symbol_table_.symbols_per_scope();
    for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
        for (const Symbol* symbol_ptr : symbol_per_scope_vector[scope])
            if (export_temps || !symbol_ptr->is_temp_variable())
                write_symbol_line(symbol_ptr);
}

void
TranslationUnit::export_within_dir(
    const std::string_view dirname,
    void (TranslationUnit::*export_func)() const) const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    const auto original_path = std::filesystem::current_path();
    create_export_directory(dirname);
    enter_export_directory(dirname);
    (this->*export_func)();
    exit_export_directory(original_path);
}

void
TranslationUnit::export_diagnostics_impl() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    const std::string outfile_name = source_path_.filename().string() + k_diagnostic_export_ext;
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(FMT::format(
            "Failed opening file {} to export compile errors", outfile_name));

    outfile << k_diagnostic_csv_export_header; // Write CSV header.
    auto write_issue_line = [&](const DiagnosticCode code, const Issue& issue)
    {
        outfile << FMT::format(
            "{0},{1},{2},{3}\n",
            to_string(code),
            issue.line(loc_tracker_).value,
            issue.column(loc_tracker_).value,
            to_string(issue.type)
        );
    };

    for (const auto& d : diagnostic_engine_.get_diagnostics())
    {
        write_issue_line(d->code, d->primary);
        for (const Issue& note : d->note_list)
            write_issue_line(d->code, note);
    }
}

void
TranslationUnit::export_ir_impl() const
{
    DEBUG_SMART_ASSERT(tried_compiling && "Not compiled anything yet, shouldn't be called");

    const std::string outfile_name = source_path_.filename().string() + k_ir_export_ext;
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(
            FMT::format("Failed opening file {} to export ir", outfile_name));

    outfile << k_ir_csv_export_header; // Write CSV header.

    auto write_ir_line = [&](const std::size_t quad_no, const Quad& q)
    {
        const auto [first_line, last_line] = loc_tracker_.find_lines(q.loc);
        std::string quad_label_str = alpha::ir::info_traits::is_branching(q.opcode)
                                     ? std::to_string(q.label)
                                     : alpha::k_not_available_marker;

        outfile << FMT::format(
            "{0},{1},{2},{3},{4},{5},{6},{7}\n",
            quad_no,
            to_string(q.opcode),
            expr_printer(q.result, k_not_available_marker),
            expr_printer(q.arg1, k_not_available_marker),
            expr_printer(q.arg2, k_not_available_marker),
            quad_label_str,
            first_line.value,
            last_line.value
        );
    };

    const auto& quads = pass_manager_->get_quads();
    for (std::size_t i = 0; i < quads.size(); ++i)
        write_ir_line(i + 1, quads[i]); // +1 cause quad address 0 is indicating no-address
}
} // namespace alpha
