#include "driver/alpha_driver.hpp"
#include <fstream> // for basic_ostream, basic_ofstream
#include <iostream>  // for cout, cerr
#include <list>      // for _List_const_iterator, list
#include <stdexcept> // for runtime_error, invalid_argument
#include <string>
#include <vector>                   // for vector
#include "alpha_parser.hpp"         // for alpha_yyparse
#include "core/konstants.hpp" // for k_global_scope
#include "core/numeric_types.hpp"     // for u32
#include "utils/cli_color.h"        // for COLOR_ASCII_BLUE, SGR_RESET
#include "utils/format_adapter.hpp" // for format, FMT
#include "utils/smart_assert.h"     // for SMART_ASSERT
#include "parser/ir.hpp"

static constexpr unsigned k_flex_eof_padding = 2;

bool g_show_parser_trace = false;

namespace // (Anonymous)
{
    std::ifstream open_alpha_source_file(const std::string &filepath);

    void create_export_directory(std::string_view dirname);

    void enter_export_directory(std::string_view dirname);

    void exit_export_directory(const std::filesystem::path &original_path);

    void expr_validator(const Alpha::Expr *e);

    std::string expr_printer(const Alpha::Expr *expr);

    template<unsigned column, unsigned column_width, typename T>
    std::string color_column(T &&value);

    template<bool colorize, unsigned column, unsigned column_width, typename T>
    std::string format_column(T &&value);

    template<bool colorize, typename Stream>
    void print_quads(Stream &out, const std::vector<Alpha::Quad> &quads,
                     const Alpha::LocationTracker &lt);

    std::ifstream open_alpha_source_file(const std::string &filepath)
    {
        if (!std::filesystem::is_regular_file(filepath))
            throw std::invalid_argument(FMT::format("{} is not a regular file.", filepath));
        std::ifstream inputFile(filepath);
        if (!inputFile)
            throw std::invalid_argument(
                FMT::format("Failed opening {} for reading.", filepath));
        return inputFile;
    }

    void create_export_directory(std::string_view dirname)
    {
        std::filesystem::create_directories(dirname);
    }

    void enter_export_directory(std::string_view dirname)
    {
        std::filesystem::current_path(dirname);
    }

    void exit_export_directory(const std::filesystem::path &original_path)
    {
        std::filesystem::current_path(original_path);
    }

    void expr_validator(const Alpha::Expr *e)
    {
        SMART_ASSERT(!!e);
        //      auto has_loc = [](const Alpha::Expr *e) -> bool { return e->location != Alpha::k_no_loc; };
        //      auto has_symbol = [](const Alpha::Expr *e) -> bool { return e->symbol; };
        //      auto has_index = [](const Alpha::Expr *e) -> bool { return e->index; };
        // //
        //     using AET = Alpha::Expr::Type;
        //     switch (e->type)
        //     {
        // #define CASE_ASSERT(...)                                                                           \
        //         SMART_ASSERT(__VA_ARGS__);                                                                 \
        //         break;
        //     case AET::ARITHMETIC_EXPR: CASE_ASSERT(has_symbol(e), has_loc(e), !has_index(e));
        //     case AET::ASSIGN_EXPR: CASE_ASSERT(has_symbol(e), has_loc(e), !has_index(e));
        //     case AET::BOOL_EXPR: CASE_ASSERT(has_symbol(e), has_loc(e), !has_index(e));
        //     case AET::CONST_BOOL: CASE_ASSERT(!has_symbol(e), has_loc(e) /* ANY_VAL */);
        //     case AET::CONST_INT: CASE_ASSERT(!has_symbol(e), has_loc(e) /* ANY_VAL */);
        //     case AET::CONST_NIL: CASE_ASSERT(!has_symbol(e), has_loc(e), !has_index(0));
        //     case AET::CONST_FLOAT: CASE_ASSERT(!has_symbol(e), has_loc(e) /* ANY_VAL */);
        //     case AET::CONST_STRING: CASE_ASSERT(!has_symbol(e), has_loc(e) /* ANY_VAL */);
        //     case AET::LIBRARY_FUNCTION: CASE_ASSERT(has_symbol(e), !has_loc(e), !has_index(e));
        //     case AET::NEW_TABLE: CASE_ASSERT(has_symbol(e), has_loc(e), !has_index(e));
        //     case AET::PROGRAM_FUNCTION: CASE_ASSERT(has_symbol(e), has_loc(e), !has_index(e));
        //     case AET::TABLE_ITEM: CASE_ASSERT(has_symbol(e), has_loc(e), has_index(e));
        //     case AET::VARIABLE: CASE_ASSERT(has_symbol(e), !has_index(e));
        //     default: throw std::logic_error(ATTACH_CONTEXT(
        //             FMT::format("BUG: Should never reach here. Type's int_value = `{}`",
        //                 static_cast<int>(e->type))));
        //     }
    }

    std::string expr_printer(const Alpha::Expr *expr)
    {
        using namespace Alpha;
        if (!expr)
            return "";
#ifdef DEBUG_MODE
        expr_validator(expr);
#endif
        using ET = Expr::Type;
        switch (expr->type)
        {
            case ET::CONST_BOOL: return static_cast<const ConstBoolExpr *>(expr)->value
                                            ? "true"
                                            : "false";
            case ET::CONST_INT: return FMT::to_string(
                    static_cast<const ConstIntExpr *>(expr)->value);
            case ET::CONST_FLOAT: return FMT::to_string(
                    static_cast<const ConstFloatExpr *>(expr)->value);
            case ET::CONST_STRING:
                return FMT::format("\"{}\"", static_cast<const ConstStringExpr *>(expr)->value);
            case ET::CONST_NIL: return "nil";
            case ET::ARITHMETIC_EXPR: return static_cast<const ArithmeticExpr *>(expr)->symbol->
                        name;
            case ET::ASSIGN_EXPR: return static_cast<const AssignExpr *>(expr)->symbol->name;
            case ET::BOOL_EXPR: return static_cast<const BoolExpr *>(expr)->symbol->name;
            case ET::LIBRARY_FUNCTION: return static_cast<const LibFuncExpr *>(expr)->symbol->name;
            case ET::NEW_TABLE: return static_cast<const NewTableExpr *>(expr)->symbol->name;
            case ET::PROGRAM_FUNCTION: return static_cast<const ProgFuncExpr *>(expr)->symbol->name;
            case ET::TABLE_ITEM: return static_cast<const TableItemExpr *>(expr)->symbol->name;
            case ET::VARIABLE: return static_cast<const VariableExpr *>(expr)->symbol->name;
            default: UNREACHABLE("Unknown Expr::Type");
        }
    }

    template<unsigned Column, unsigned ColumnWidth, typename T>
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

    template<bool Colorize, unsigned Column, unsigned ColumnWidth, typename T>
    std::string format_column(T &&value)
    {
        if (Colorize)
            return color_column<Column, ColumnWidth>(std::forward<T>(value));
        return FMT::format("{:<{}}", std::forward<T>(value), ColumnWidth);
    }

    template<bool Colorize, typename Stream>
    void print_quads(Stream &out, const std::vector<Alpha::Quad> &quads,
                     const Alpha::LocationTracker &lt)
    {
        constexpr Alpha::u32 widths[] = {10, 15, 20, 20, 20, 10, 10};
        constexpr Alpha::u32 quad_header_width = [&widths]() constexpr
        {
            Alpha::u32 width = 0;
            for (Alpha::u32 i = 0; i < std::size(widths); ++i)
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
        out << std::string(quad_header_width, '-') << std::endl;

        // Write quads.
        const auto quads_size = quads.size();
        for (Alpha::u32 i = 0; i < quads_size; i++)
        {
            const Alpha::Quad &q = quads[i];

            auto quad_line_num = lt.find_first_line(q.location);
            std::string quad_line_str = (quad_line_num == Alpha::k_no_line)
                                            ? Alpha::k_not_available_marker
                                            : std::to_string(quad_line_num);
            std::string quad_label_str = (q.label == Alpha::k_no_label)
                                             ? Alpha::k_not_available_marker
                                             : std::to_string(q.label);

            out << FMT::format("{} {} {} {} {} {} {}\n",
                               format_column<Colorize, 0, widths[0]>(i + 1),
                               format_column<Colorize, 1, widths[1]>(to_string(q.iopcode)),
                               format_column<Colorize, 2, widths[2]>(expr_printer(q.result)),
                               format_column<Colorize, 3, widths[3]>(expr_printer(q.arg1)),
                               format_column<Colorize, 4, widths[4]>(expr_printer(q.arg2)),
                               format_column<Colorize, 5, widths[5]>(quad_label_str),
                               format_column<Colorize, 6, widths[6]>(quad_line_str) //
            );
        }
        if constexpr (Colorize)
            out << SGR_RESET;
        out << std::endl;
    }
} // namespace

namespace Alpha
{
    Driver::Driver(const std::string &source_filepath, bool show_parser_trace)
        : source_filepath_(source_filepath),
          // Convert std::string to std::filesystem::path implicitly
          flex_buffer_(source_filepath),
          lt_(flex_buffer_.size() - k_flex_eof_padding),
          diagnostic_engine_(),
          st_(),
          lexer_ctx_(source_filepath),
          parse_ctx_(st_, diagnostic_engine_),
          semantic_driver_(SemanticDriver::Options{true, true, true}, &parse_ctx_, &st_,
                           &diagnostic_engine_)
    {
        // TODO INITIALIZE SEMANTIC OPTS CORRECTLY.
        g_show_parser_trace = show_parser_trace;
    }

    Driver::~Driver()
    {
        alpha_yylex_destroy();
    }

    void Driver::run_alpha_parser()
    {
        parser_retval_ = alpha_yyparse(lt_, diagnostic_engine_, lexer_ctx_, semantic_driver_);
    }

    void Driver::show_symbol_table() const
    {
        std::cout << COLOR_ASCII_BLUE;
        const auto &symbol_per_scope_vector = st_.symbols_per_scope();
        for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
        {
            if (symbol_per_scope_vector[scope].empty())
                continue;
            std::cout << FMT::format("----------------------------     Scope #{:<4}     "
                                     "----------------------------\n",
                                     scope);
            for (auto symbol_ptr: symbol_per_scope_vector[scope])
                std::cout << FMT::format("{:<30} {:<20} (line {:>5}) (scope {:>4})\n",
                                         FMT::format("\"{}\"", symbol_ptr->name),
                                         FMT::format("[{}]", symbol_ptr->type_to_string()),
                                         lt_.find_symbol_line(symbol_ptr->loc),
                                         symbol_ptr->scope);
            std::cout << std::endl;
        }
        std::cout << SGR_RESET << std::endl;
    }

    void Driver::show_compile_issues() const
    {
        const std::string source_filename = source_filepath_.filename().string();

        for (const auto &cti: diagnostic_engine_.get_compile_time_issues())
            std::cerr << cti->make_pretty_diagnostic(source_filename, lt_,
                                                     flex_buffer_.const_buffer());
    }

    void Driver::show_quads() const
    {
        // if (et_.contain_errors()) // We dont show quads if there are errors.
        // return;
        // TODO!! UNCOMMENT!
        print_quads<true>(std::cout, semantic_driver_.retrieve_quads(), lt_);
    }

    void Driver::export_symbol_table() const
    {
        export_within_dir(k_symbol_table_exports_dirname, &Driver::export_symbol_table_impl);
    }

    void Driver::export_compile_errors() const
    {
        export_within_dir(k_compile_error_exports_dirname, &Driver::export_compile_errors_impl);
    }

    void Driver::export_quads() const
    {
        if (diagnostic_engine_.contain_fatal_errors() || diagnostic_engine_.contain_errors())
            return;
        export_within_dir(k_quad_exports_dirname, &Driver::export_quads_impl);
    }

    Driver::FlexBuffer::FlexBuffer(const std::string &input_filepath)
    {
        std::ifstream input_file(open_alpha_source_file(input_filepath));
        const std::size_t input_file_size = std::filesystem::file_size(input_filepath);

        const std::size_t scan_buffer_size = input_file_size + k_flex_eof_padding;
        buffer_ = std::make_unique<char[]>(scan_buffer_size);

        if (!input_file.read(buffer_.get(), input_file_size))
            throw std::runtime_error(ATTACH_CONTEXT("BUG: Failed reading input_file."));

        // Flex requires two NULL-bytes at the end of the buffer (End-Of-Buffer marker).
        buffer_[input_file_size] = buffer_[input_file_size + 1] = '\0';
        size_ = scan_buffer_size;
        state_ = alpha_yy_scan_buffer(buffer_.get(), size_);
        if (state_ == nullptr)
            throw std::runtime_error(ATTACH_CONTEXT(
                FMT::format("BUG: Failed to initialize Flex scanner for file `{}`:"
                    "alpha_yy_scan_buffer returned nullptr (buffer-size = `{}`).",
                    input_filepath, std::to_string(size_))));
    }

    Driver::FlexBuffer::~FlexBuffer()
    {
        // Class invariant: `state_` must be non-null after construction.
        // Violations indicate a serious logic issue (e.g., double-deletion or moved-from object).
        SMART_ASSERT(!!state_);
        state_ = nullptr;
        // We nullified our reference to YY_BUFFER_STATE
        // Flex has it own, also we DID NOT use alpha_yy_delete_buffer()
        // So we can call alpha_yylex_destroy() on driver, which call delete_buffer()
        // and also deletes its own stack.
    }

    void Driver::export_symbol_table_impl() const
    {
        const std::string outfile_name = source_filepath_.filename().string() + ".st.csv";
        std::ofstream outfile(outfile_name);
        if (!outfile)
            throw std::runtime_error(
                FMT::format("Failed opening file {} to export symbol table", outfile_name));

        outfile << k_symbol_table_csv_export_header; // Write CSV header.

        auto write_symbol = [&](const Symbol *symbol_ptr)
        {
            outfile << FMT::format(
                "{},{},{},{}\n", symbol_ptr->name, symbol_ptr->type_to_string(),
                lt_.find_symbol_line(symbol_ptr->loc), symbol_ptr->scope);
        };

        const auto &symbol_per_scope_vector = st_.symbols_per_scope();
        for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
            for (const Symbol *symbol_ptr: symbol_per_scope_vector[scope])
                write_symbol(symbol_ptr);
    }

    void Driver::export_within_dir(std::string_view dirname,
                                   void (Driver::*export_func)() const) const
    {
        const auto original_path = std::filesystem::current_path();
        create_export_directory(dirname);
        enter_export_directory(dirname);
        (this->*export_func)();
        exit_export_directory(original_path);
    }

    void Driver::export_compile_errors_impl() const
    {
        const std::string outfile_name = source_filepath_.filename().string() + ".issue.csv";
        std::ofstream outfile(outfile_name);
        if (!outfile)
            throw std::runtime_error(FMT::format(
                "Failed opening file {} to export compile errors", outfile_name));

        outfile << k_error_csv_export_header; // Write CSV header.
        auto write_diagnostic = [&](const Issue &diag)
        {
            outfile << FMT::format("{},{},{},{}\n", diag.line(lt_), diag.column(lt_),
                                   diag.type_to_string(), diag.desc);
        };

        for (const auto &cti: diagnostic_engine_.get_compile_time_issues())
        {
            write_diagnostic(cti->primary);
            for (const Issue &note: cti->notes)
                write_diagnostic(note);
        }
    }

    void Driver::export_quads_impl() const
    {
        const std::string outfile_name = source_filepath_.filename().string() + ".quads";
        std::ofstream outfile(outfile_name);
        if (!outfile)
            throw std::runtime_error(
                FMT::format("Failed opening file {} to export quads", outfile_name));

        // print_quads<false>(outfile, parse_ctx_.quad_handler.quads(), lt_);
    }
} // namespace Alpha
