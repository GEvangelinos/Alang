#include <alpha_parser.gen.hpp>
#include <fstream>
#include <driver/translation_unit.hpp>
#include "driver/alpha_driver.hpp"
#include "driver/alpha_driver_exceptions.hpp"
#include "scanner/alpha_scanner.gen.hpp"
#include "utils/cli_color.h"

namespace
{
void create_export_directory(std::string_view dirname);
void enter_export_directory(std::string_view dirname);
void exit_export_directory(const std::filesystem::path &original_path);
std::string expr_printer(const alpha::Expr *expr);

template<unsigned column,unsigned column_width,typename T>
std::string color_column(T &&value);
template<bool colorize,unsigned column,unsigned column_width,typename T>
std::string format_column(T &&value);
template<bool colorize,typename Stream>
void print_quads(
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

std::string expr_printer(const alpha::Expr *expr)
{
    using namespace alpha;
    if (!expr)
        return "";
    using ET = Expr::Type;
    switch (expr->type)
    {
    case ET::CONST_BOOL: return static_cast<const ConstBoolExpr *>(expr)->value ? "true" : "false";
    case ET::CONST_INT: return FMT::to_string(static_cast<const ConstIntExpr *>(expr)->value);
    case ET::CONST_FLOAT: return FMT::to_string(static_cast<const ConstFloatExpr *>(expr)->value);
    case ET::CONST_STRING:
        return FMT::format("\"{}\"", static_cast<const ConstStringExpr *>(expr)->value);
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

template<unsigned Column,unsigned ColumnWidth,typename T>
std::string color_column(T &&value)
{
    constexpr unsigned column_count = 7;
    static_assert(Column < column_count, "So far we support a maximum of 7 columns");

    const char *ascii_color;
        // clang-format off
                switch (Column)
                {
                case 0:  ascii_color = COLOR_ASCII_WHITE;   break;
                case 1:  ascii_color = COLOR_ASCII_RED;     break;
                case 2:  ascii_color = COLOR_ASCII_GREEN;   break;
                case 3:  ascii_color = COLOR_ASCII_BLUE;    break;
                case 4:  ascii_color = COLOR_ASCII_CYAN;    break;
                case 5:  ascii_color = COLOR_ASCII_MAGENTA; break;
                case 6:  ascii_color = COLOR_ASCII_YELLOW;  break;
                default: ascii_color = COLOR_ASCII_DEFAULT; break;
                }
    // clang-format on
    return FMT::format("{}{:<{}}{}", ascii_color, std::forward<T>(value), ColumnWidth,
                       SGR_RESET);
}

template<bool Colorize,unsigned Column,unsigned ColumnWidth,typename T>
std::string format_column(T &&value)
{
    if (Colorize)
        return color_column<Column, ColumnWidth>(std::forward<T>(value));
    return FMT::format("{:<{}}", std::forward<T>(value), ColumnWidth);
}

template<bool Colorize,typename Stream>
void print_quads(Stream &out, const std::vector<alpha::Quad> &quads,
                 const alpha::LocationTracker &lt)
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

    // Write export header.
    out << FMT::format("{} {} {} {} {} {} {}\n",
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

        const auto quad_line_num = lt.find_first_line(q.location);
        std::string quad_line_str = quad_line_num == alpha::k_no_line
                                    ? alpha::k_not_available_marker
                                    : std::to_string(quad_line_num);
        std::string quad_label_str = alpha::ir::info_traits::is_branching(quads[i].opcode)
                                     ? std::to_string(q.label)
                                     : alpha::k_not_available_marker;

        out << FMT::format(
            "{} {} {} {} {} {} {}\n",
            format_column<Colorize, 0, widths[0]>(i + 1),
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
    TUBuffer &tu_buffer,
    LocationTracker &lt,
    DiagnosticEngine &diagnostic_engine,
    SymbolTable *const symbol_table)
    : lt_(lt),
      diagnostic_engine_(diagnostic_engine),
      scanner_handle_(tu_buffer),
      parse_ctx_(utils::require_ptr(symbol_table)),
      lexer_ctx_(),
      semantic_system_(
          ss_options_, &parse_ctx_, utils::require_ptr(symbol_table), &diagnostic_engine_.reporter
      ) {}

void
PassManager::execute() { run_frontend(); }

void
PassManager::run_frontend()
{
    running_phase_ = Phase::FRONTEND;
    parser_retval_ = alpha_yyparse(
        scanner_handle_.get(),
        lexer_ctx_,
        lt_,
        diagnostic_engine_,
        diagnostic_engine_.reporter,
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
    case Phase::FRONTEND: return semantic_system_.status_gateway.notify_hard_error();
    default: UNREACHABLE("Unknown `running_phase`");
    }
}

DiagnosticEngine::Policy
TranslationUnit::create_diagnostic_engine_policy()
{
    return DiagnosticEngine::Policy{
        .should_emit_diagnostic = [this]() { return pass_manager_->is_in_hard_error(); },
        .notify_max_errors_reached = [this]() { notify_max_errors_reached(); },
        .notify_fatal_error = [this]() { notify_fatal_error(); },
        .notify_hard_error = [this]() { pass_manager_->notify_hard_error(); }
    };
}

TUBuffer::TUBuffer(
    const std::filesystem::path &path,
    const std::size_t null_padding)
    : null_padding(null_padding)
{
    std::ifstream ifs = open_source(path);

    const auto filesize = std::filesystem::file_size(path);
    // TODO: specify filesize.. If C++ doesnt have a way to convert bytes to KB,MB,GB
    // You have a conversion function in MicroTCP's project code.
    if (filesize > k_max_input_file_size)
        throw std::invalid_argument(FMT::format("file {} is too big.", path.string()));

    const auto tub_size = filesize + null_padding;
    data_ = std::make_unique<char[]>(tub_size);
    size_ = tub_size;

    if (!ifs.read(data_.get(), filesize))
        throw std::invalid_argument(
            FMT::format("Failed reading source file: {}", path.string()));

    // Flex requires two NULL-bytes at the end of the buffer (End-Of-Buffer marker).
    for (auto i = filesize; i < tub_size; ++i)
        data_[i] = '\0';
}

std::ifstream
TUBuffer::open_source(const std::filesystem::path &path)
{
    if (!std::filesystem::is_regular_file(path))
        throw std::invalid_argument(FMT::format("{} is not a regular file.", path.string()));
    if (std::ifstream ifs(path); ifs)
        return ifs;
    throw std::invalid_argument(FMT::format("Failed opening {} for reading.", path.string()));
}

TranslationUnit::TranslationUnit(
    const std::filesystem::path &source_path,
    CompilationOptions::Values comp_options)
    : source_path_(source_path),
      compilation_options_(std::move(comp_options)),
      tu_buffer_(source_path, k_scanner_eof_null_padding),
      loc_tracker_(tu_buffer_.size() - tu_buffer_.null_padding),
      diagnostic_engine_(create_diagnostic_engine_policy(), comp_options.max_errors),
      diagnostic_formatter_(source_path, loc_tracker_, tu_buffer_.data()),
      symbol_table_(),
      pass_manager_(std::make_unique<PassManager>(
          tu_buffer_, loc_tracker_, diagnostic_engine_, &symbol_table_)) {}

PassManager::ScannerHandle::ScannerHandle(TUBuffer &tu_buffer)
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
        compiled_ok_ = true;
    }
    catch (exceptions::DiagnosticFatalError) {}
    catch (exceptions::DiagnosticErrorLimitExceeded) {}
    catch (exceptions::SanityLimitExceededError &e)
    {
        std::cerr << FMT::format("Exception caught: {}", e.what()) << std::endl;
    }
}

void
TranslationUnit::notify_fatal_error() { throw exceptions::DiagnosticFatalError(); }

void
TranslationUnit::notify_max_errors_reached() { throw exceptions::DiagnosticErrorLimitExceeded(); }

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
TranslationUnit::show_compile_issues() const
{
    const std::string source_filename = source_path_.filename().string();

    for (const auto &diagnostic: diagnostic_engine_.get_diagnostics())
        std::cerr << "CALL FORMATTER!";

    std::cerr << std::endl;
}

void
TranslationUnit::show_quads() const
{
    // TODO!! UNCOMMENT!
    // if (diagnostic_engine_.has_errors())
    //     return;
    print_quads<true>(std::cout, pass_manager_->get_quads(), loc_tracker_);
}

void
TranslationUnit::export_symbol_table() const
{
    export_within_dir(k_symbol_table_exports_dirname, &TranslationUnit::export_symbol_table_impl);
}

void
TranslationUnit::export_compile_errors() const
{
    export_within_dir(k_compile_error_exports_dirname,
                      &TranslationUnit::export_compile_errors_impl);
}

void
TranslationUnit::export_quads() const
{
    if (diagnostic_engine_.has_fatal_errors() || diagnostic_engine_.has_errors())
        return;
    export_within_dir(k_quad_exports_dirname, &TranslationUnit::export_quads_impl);
}

void
TranslationUnit::export_symbol_table_impl() const
{
    const std::string outfile_name = source_path_.filename().string() + ".st.csv";
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(
            FMT::format("Failed opening file {} to export symbol table", outfile_name));

    outfile << k_symbol_table_csv_export_header; // Write CSV header.

    auto write_symbol = [&](const Symbol *symbol_ptr)
    {
        outfile << FMT::format(
            "{},{},{},{}\n", symbol_ptr->name, symbol_ptr->type_to_string(),
            loc_tracker_.find_symbol_line(symbol_ptr->loc), symbol_ptr->scope);
    };

    const auto &symbol_per_scope_vector = symbol_table_.symbols_per_scope();
    for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
        for (const Symbol *symbol_ptr: symbol_per_scope_vector[scope])
            write_symbol(symbol_ptr);
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
TranslationUnit::export_compile_errors_impl() const
{
    const std::string outfile_name = source_path_.filename().string() + ".issue.csv";
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(FMT::format(
            "Failed opening file {} to export compile errors", outfile_name));

    outfile << k_error_csv_export_header; // Write CSV header.
    auto write_diagnostic = [&](const Issue &diag)
    {
        outfile << FMT::format("{0},{1},{2},{3}\n",
                               diag.line(loc_tracker_),
                               diag.column(loc_tracker_),
                               to_string(diag.type),
                               diag.desc
        );
    };

    for (const auto &cti: diagnostic_engine_.get_diagnostics())
    {
        write_diagnostic(cti->primary);
        for (const Issue &note: cti->note_list)
            write_diagnostic(note);
    }
}

void
TranslationUnit::export_quads_impl() const
{
    const std::string outfile_name = source_path_.filename().string() + ".quads";
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(
            FMT::format("Failed opening file {} to export quads", outfile_name));

    print_quads<false>(outfile, pass_manager_->get_quads(), loc_tracker_);
}
} // namespace alpha
