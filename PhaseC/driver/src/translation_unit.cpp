#include <alpha_parser.gen.hpp>
#include <fstream>
#include <driver/translation_unit.hpp>

#include "core/konstants.hpp"
#include "driver/alpha_driver.hpp"
#include "driver/exception.hpp"
#include "driver/konstants.hpp"
#include "core/exception.hpp"
#include "scanner/alpha_scanner.gen.hpp"
#include "support/cli_color.h"

#define WARNING_BANNER_PREFIX COLOR_ASCII_MAGENTA SGR_BOLD "Warning" SGR_BLINK "❗" SGR_RESET ": "
inline constexpr auto k_reescaped_warning_banner =
        WARNING_BANNER_PREFIX
        "Special characters are escaped; Ex.: \"\\n\" prints literally (not a newline) to keep the table aligned.\n"
        SGR_RESET;

inline constexpr auto k_temp_reuse_warning_banner =
        WARNING_BANNER_PREFIX
        "Temporaries are context-local. Each function starts with a fresh temp frame (counter reset to zero).\n"
        "           Nested function definitions or functions assigned to tables/variables therefore allocate temps from a\n"
        "           clean slate. When the function definition ends, the previous temp counter is restored.\n"
        SGR_RESET;

inline constexpr auto k_cya_mode_off_warning_banner =
#ifdef CYA_MODE
        ""
#else
        WARNING_BANNER_PREFIX
        "This executable was built with CYA_MODE disabled. As a result:\n"
        "           a) Aggressive reuse of temporary variables is enabled; table literals are handled differently.\n"
        "           b) Assignment temps are no longer created except when strictly required (e.g., inside function calls).\n"
        SGR_RESET
#endif
;

namespace
{
void create_export_directory(std::string_view dirname);
void enter_export_directory(std::string_view dirname);
void exit_export_directory(const std::filesystem::path &original_path);
std::string expr_printer(const alpha::Expr *expr, const char *missing_marker = "");

template<unsigned column, unsigned column_width, typename T>
std::string color_column(T &&value);
template<bool colorize, unsigned column, unsigned column_width, typename T>
std::string format_column(T &&value);
template<bool colorize, typename Stream>
void print_ir(
    Stream &out, const std::vector<alpha::Quad> &quads, const alpha::LocationTracker &lt);

void create_export_directory(const std::string_view dirname)
{
    std::filesystem::create_directories(dirname);
}

void enter_export_directory(std::string_view dirname) { std::filesystem::current_path(dirname); }

void exit_export_directory(const std::filesystem::path &original_path)
{
    std::filesystem::current_path(original_path);
}

std::string escape(const char *const str)
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

std::string expr_printer(const alpha::Expr *expr, const char *const missing_marker)
{
    using namespace alpha;
    if (!expr)
        return missing_marker;
    using ET = Expr::Type;
    switch (expr->type)
    {
    case ET::CONST_BOOL: return static_cast<const ConstBoolExpr *>(expr)->value ? "true" : "false";
    case ET::CONST_INT: return std::to_string(static_cast<const ConstIntExpr *>(expr)->value);
    case ET::CONST_FLOAT: return std::to_string(static_cast<const ConstFloatExpr *>(expr)->value);
    case ET::CONST_STRING:
        return FMT::format("\"{}\"", escape(static_cast<const ConstStringExpr *>(expr)->value));
    case ET::CONST_NIL: return "nil";
    case ET::ARITHMETIC_EXPR: return static_cast<const ArithmeticExpr *>(expr)->var_symbol->name;
    case ET::ASSIGN_EXPR: return static_cast<const AssignExpr *>(expr)->var_symbol->name;
    case ET::BOOL_EXPR: return static_cast<const BoolExpr *>(expr)->var_symbol->name;
    case ET::LIBRARY_FUNCTION: return static_cast<const LibFuncExpr *>(expr)->func_symbol->name;
    case ET::NEW_TABLE: return static_cast<const NewTableExpr *>(expr)->var_symbol->name;
    case ET::PROGRAM_FUNCTION: return static_cast<const ProgFuncExpr *>(expr)->func_symbol->name;
    case ET::TABLE_ITEM: return static_cast<const TableItemExpr *>(expr)->var_symbol->name;
    case ET::VARIABLE: return static_cast<const VariableExpr *>(expr)->var_symbol->name;
    default:
        UNREACHABLE(FMT::format("Unhandled Expr::Type: int({}) = {}",
            TO_STRING(expr->type), static_cast<int>(expr->type)
        ));
    }
}

template<unsigned Column, unsigned ColumnWidth, typename T>
std::string color_column(T &&value)
{
    constexpr unsigned column_count = 7;
    static_assert(Column < column_count, "So far we support a maximum of 7 columns");

    const char *ascii_color;
    if constexpr (Column == 0) ascii_color = COLOR_ASCII_WHITE;
    else if constexpr (Column == 1) ascii_color = COLOR_ASCII_RED;
    else if constexpr (Column == 2) ascii_color = COLOR_ASCII_GREEN;
    else if constexpr (Column == 3) ascii_color = COLOR_ASCII_BLUE;
    else if constexpr (Column == 4) ascii_color = COLOR_ASCII_CYAN;
    else if constexpr (Column == 5) ascii_color = COLOR_ASCII_YELLOW;
    else if constexpr (Column == 6) ascii_color = COLOR_ASCII_MAGENTA;
    else ascii_color = COLOR_ASCII_DEFAULT;
    return FMT::format("{}{:<{}}{}", ascii_color, std::forward<T>(value), ColumnWidth, SGR_RESET);
}

template<bool Colorize, unsigned Column, unsigned ColumnWidth, typename T>
std::string format_column(T &&value)
{
    if constexpr (Colorize)
        return color_column<Column, ColumnWidth>(std::forward<T>(value));
    else
        return FMT::format("{:<{}}", std::forward<T>(value), ColumnWidth);
}

template<bool Colorize, typename Stream>
void print_ir(Stream &out, const std::vector<alpha::Quad> &quads, const alpha::LocationTracker &lt)
{
    constexpr alpha::u32 widths[] = {10, 15, 20, 20, 20, 10, 10};
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
        "{0} {1} {2} {3} {4} {5} {6}\n",
        format_column<Colorize, 0, widths[0]>("quad#"),
        format_column<Colorize, 1, widths[1]>("opcode"),
        format_column<Colorize, 2, widths[2]>("result"),
        format_column<Colorize, 3, widths[3]>("arg1"),
        format_column<Colorize, 4, widths[4]>("arg2"),
        format_column<Colorize, 5, widths[5]>("label"),
        format_column<Colorize, 6, widths[6]>("line")
    );

    // Write separating dash line.
    out << std::string(quad_header_width, '-') << '\n';

    // Write quads.
    const auto quads_size = quads.size();
    for (alpha::u32 i = 0; i < quads_size; i++)
    {
        const alpha::Quad &q = quads[i];

        std::string quad_label_str = alpha::ir::info_traits::is_branching(quads[i].opcode)
                                     ? std::to_string(q.label)
                                     : alpha::k_not_available_pretty_marker;

        const auto [first_line, last_line] = lt.find_lines(q.location);
        std::string quad_line_str = first_line == last_line
                                    ? std::to_string(first_line)
                                    : FMT::format("{}-{}", first_line, last_line);

        out << FMT::format(
            "{} {} {} {} {} {} {}\n",
            format_column<Colorize, 0, widths[0]>(i + 1), // +1 as, 0 is indicating no-address
            format_column<Colorize, 1, widths[1]>(to_string(q.opcode)),
            format_column<Colorize, 2, widths[2]>(expr_printer(q.result)),
            format_column<Colorize, 3, widths[3]>(expr_printer(q.arg1)),
            format_column<Colorize, 4, widths[4]>(expr_printer(q.arg2)),
            format_column<Colorize, 5, widths[5]>(quad_label_str),
            format_column<Colorize, 6, widths[6]>(quad_line_str)
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
    const alpha::settings::ExprOpts &expr_opts,
    TranslationUnitBuffer &tu_buffer,
    LocationTracker &lt,
    DiagnosticEngine &diagnostic_engine,
    SymbolTable *const symbol_table)
    : lt_(lt),
      diagnostic_engine_(diagnostic_engine),
      scanner_handle_(tu_buffer),
      parse_ctx_(support::require_ptr(symbol_table)),
      lexer_ctx_(),
      semantic_system_(
          expr_opts,
          &parse_ctx_,
          support::require_ptr(symbol_table),
          diagnostic_engine_.reporter.get()
      ) {}

void
PassManager::execute() { run_frontend(); }

void
PassManager::run_frontend()
{
    // The following are the possible return values yyparse can return (based on bison's manual).
    [[maybe_unused]] constexpr auto successful_parsing = 0;
    [[maybe_unused]] constexpr auto invalid_input = 1;
    [[maybe_unused]] constexpr auto memory_exhaustion = 2;

    running_phase_ = Phase::FRONTEND;
    parser_retval_ = alpha_yyparse(
        scanner_handle_.get(),
        lexer_ctx_,
        lt_,
        diagnostic_engine_,
        *diagnostic_engine_.reporter.get(),
        semantic_system_
    );
}

bool
PassManager::is_in_hard_error()
{
    switch (running_phase_)
    {
    case Phase::FRONTEND: return semantic_system_.good();
    default: UNREACHABLE("Unknown `Phase`");
    }
}

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
        .notify_fatal_error = []() { notify_fatal_error(); },
        .notify_hard_error = [this]() { pass_manager_->notify_hard_error(); }
    };
}

TranslationUnitBuffer::TranslationUnitBuffer(
    const std::filesystem::path &path,
    const std::size_t null_padding)
    : null_padding(null_padding)
{
    std::ifstream ifs = open_source(path);
    const auto filesize = std::filesystem::file_size(path);
    const auto tub_size = filesize + null_padding;
    data_ = std::make_unique<char[]>(tub_size);
    size_ = tub_size;

    if (!ifs.read(data_.get(), filesize))
        throw alpha::exception::FileReadError(path.string());

    // Flex requires two NULL-bytes at the end of the buffer (End-Of-Buffer marker).
    for (auto i = filesize; i < tub_size; ++i)
        data_[i] = '\0';
}

std::ifstream
TranslationUnitBuffer::open_source(const std::filesystem::path &path)
{
    using FOMode = alpha::exception::FileOpenError::Mode;
    if (!std::filesystem::exists(path))
        throw alpha::exception::FileNotFoundError(path.string());
    if (std::filesystem::is_directory(path))
        throw alpha::exception::FileIsADirectoryError(path.string());
    if (!std::filesystem::is_regular_file(path))
        throw alpha::exception::FileNotRegularError(path.string());
    if (const auto filesize = std::filesystem::file_size(path); filesize > k_max_source_filesize)
        throw alpha::exception::FileTooLargeError(path.string(), filesize, k_max_source_filesize);
    std::ifstream ifs(path);
    if (!ifs)
        throw alpha::exception::FileOpenError(path.string(), FOMode::READ);
    return ifs;
}

TranslationUnit::TranslationUnit(
    const std::filesystem::path &source_path,
    const std::size_t max_errors,
    const settings::ExprOpts &expr_opts)
    : source_path_(source_path),
      expr_opts_(expr_opts),
      diagnostic_engine_(create_diagnostic_engine_policy(), max_errors),
      translation_unit_buffer_(source_path, k_scanner_eof_null_padding),
      loc_tracker_(translation_unit_buffer_.size() - translation_unit_buffer_.null_padding),
      diagnostic_formatter_(source_path, loc_tracker_, translation_unit_buffer_.data()),
      symbol_table_(),
      pass_manager_(std::make_unique<PassManager>(
          expr_opts,
          translation_unit_buffer_,
          loc_tracker_,
          diagnostic_engine_,
          &symbol_table_
      )) {}

PassManager::ScannerHandle::ScannerHandle(TranslationUnitBuffer &tu_buffer)
{
    if (alpha_yylex_init(&scanner_) != 0)
        throw std::runtime_error(ATTACH_CONTEXT("Failed to initializing scanner"));

    if (alpha_yy_scan_buffer(tu_buffer.data(), tu_buffer.size(), scanner_) == nullptr)
    {
        std::string error =
                "Failed to load Flex buffer. A common cause is forgetting "
                "to append two null bytes for padding in at end of the buffer.";
        if (alpha_yylex_destroy(scanner_) != 0)
            error += " | Additionally, cleanup of the scanner failed.";
        throw std::runtime_error(ATTACH_CONTEXT(error));
    }
}

PassManager::ScannerHandle::~ScannerHandle()
{
    DEBUG_SMART_ASSERT_EVAL(alpha_yylex_destroy(scanner_) == 0);
}

void
TranslationUnit::compile()
{
    try
    {
        pass_manager_->execute();
        // If we reach this point, execute() completed without throwing any exceptions.
        execution_completed_ = true;
    }
    catch (exception::SanityLimitError &e)
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
    std::cout << COLOR_ASCII_BLUE;
    const auto &symbol_per_scope_vector = symbol_table_.symbols_per_scope();
    for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
    {
        if (symbol_per_scope_vector[scope].empty())
            continue;
        std::cout << FMT::format(
            "----------------------------     Scope #{:<4}     ----------------------------\n",
            scope);
        for (const auto symbol_ptr: symbol_per_scope_vector[scope])
            std::cout << FMT::format("{:<30} {:<20} (line {:>5}) (scope {:>4})\n",
                                     FMT::format("\"{}\"", symbol_ptr->name),
                                     FMT::format("[{}]", symbol_ptr->type_to_string()),
                                     loc_tracker_.find_symbol_line(symbol_ptr->loc),
                                     symbol_ptr->scope);
        std::cout << '\n';
    }
    std::cout << SGR_RESET << std::endl;
}

void
TranslationUnit::show_diagnostics() const
{
    const std::string source_filename = source_path_.filename().string();

    for (const auto &diagnostic: diagnostic_engine_.get_diagnostics())
        std::cerr << diagnostic_formatter_.format_diagnostic(*diagnostic.get(), true);

    std::cerr << std::endl;
}

void
TranslationUnit::show_ir() const
{
    print_ir<true>(std::cout, pass_manager_->get_quads(), loc_tracker_);
}

void
TranslationUnit::export_symbol_table() const
{
    export_within_dir(k_symbol_table_exports_dirname,
                      &TranslationUnit::export_symbol_table_dispatch);
}

void
TranslationUnit::export_symbol_table_without_temps() const
{
    export_within_dir(k_symbol_table_exports_dirname,
                      &TranslationUnit::export_symbol_table_without_temps_dispatch);
}

void
TranslationUnit::export_diagnostics() const
{
    export_within_dir(k_diagnostic_exports_dirname,
                      &TranslationUnit::export_diagnostics_impl);
}

void
TranslationUnit::export_ir() const
{
    export_within_dir(k_ir_exports_dirname, &TranslationUnit::export_ir_impl);
}

bool TranslationUnit::compiled_ok() const noexcept
{
    return execution_completed_ && !diagnostic_engine_.has_errors();
}

void
TranslationUnit::export_symbol_table_dispatch() const { export_symbol_table_impl(true); }

void
TranslationUnit::export_symbol_table_without_temps_dispatch() const
{
    export_symbol_table_impl(false);
}

void
TranslationUnit::export_symbol_table_impl(const bool export_temps) const
{
    const std::string outfile_name = source_path_.filename().string() + k_symbol_table_export_ext;
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(
            FMT::format("Failed opening file {} to export symbol table", outfile_name));

    outfile << k_symbol_table_csv_export_header; // Write CSV header.

    auto write_symbol_line = [&](const Symbol *symbol_ptr)
    {
        outfile << FMT::format(
            "{0},{1},{2},{3}\n",
            symbol_ptr->name,
            symbol_ptr->type_to_string(),
            loc_tracker_.find_symbol_line(symbol_ptr->loc),
            symbol_ptr->scope
        );
    };

    const auto &symbol_per_scope_vector = symbol_table_.symbols_per_scope();
    for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
        for (const Symbol *symbol_ptr: symbol_per_scope_vector[scope])
            if (export_temps || !symbol_ptr->is_temp_variable())
                write_symbol_line(symbol_ptr);
}

void
TranslationUnit::export_within_dir(
    const std::string_view dirname,
    void (TranslationUnit::*export_func)() const) const
{
    const auto original_path = std::filesystem::current_path();
    create_export_directory(dirname);
    enter_export_directory(dirname);
    (this->*export_func)();
    exit_export_directory(original_path);
}

void
TranslationUnit::export_diagnostics_impl() const
{
    const std::string outfile_name = source_path_.filename().string() + k_diagnostic_export_ext;
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(FMT::format(
            "Failed opening file {} to export compile errors", outfile_name));

    outfile << k_diagnostic_csv_export_header; // Write CSV header.
    auto write_issue_line = [&](const DiagnosticCode code, const Issue &issue)
    {
        outfile << FMT::format(
            "{0},{1},{2},{3}\n",
            to_string(code),
            issue.line(loc_tracker_),
            issue.column(loc_tracker_),
            to_string(issue.type)
        );
    };

    for (const auto &d: diagnostic_engine_.get_diagnostics())
    {
        write_issue_line(d->code, d->primary);
        for (const Issue &note: d->note_list)
            write_issue_line(d->code, note);
    }
}

void
TranslationUnit::export_ir_impl() const
{
    const std::string outfile_name = source_path_.filename().string() + k_ir_export_ext;
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(
            FMT::format("Failed opening file {} to export ir", outfile_name));

    outfile << k_ir_csv_export_header; // Write CSV header.

    auto write_ir_line = [&](const std::size_t quad_no, const Quad &q)
    {
        const auto [first_line, last_line] = loc_tracker_.find_lines(q.location);
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
            first_line,
            last_line
        );
    };

    const auto &quads = pass_manager_->get_quads();
    for (std::size_t i = 0; i < quads.size(); ++i)
        write_ir_line(i + 1, quads[i]); // +1 cause quad address 0 is indicating no-address
}
} // namespace alpha
