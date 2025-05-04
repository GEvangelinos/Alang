#include "alpha_driver.hpp"
#include <fstream>                   // for basic_ostream, basic_ofstream
#include <iostream>                  // for cout, cerr
#include <list>                      // for _List_const_iterator, list
#include <stdexcept>                 // for runtime_error, invalid_argument
#include <vector>                    // for vector
#include "alpha_parser.hpp"          // for alpha_yyparse
#include "core/alpha_konstants.hpp"  // for k_global_scope
#include "core/alpha_types.hpp"      // for u32
#include "utils/cli_color.h"         // for COLOR_ASCII_FG_BLUE, SGR_RESET
#include "utils/format_adapter.hpp"  // for format, fmt_ns
#include "utils/smart_assert.h"      // for SMART_ASSERT

static constexpr unsigned k_flex_eof_padding = 2;

bool g_show_parser_trace = false;

namespace // (Anonymous)
{
        std::ifstream open_alpha_source_file(const std::string &filepath)
        {
                if (!std::filesystem::is_regular_file(filepath))
                        throw std::invalid_argument(fmt_ns::format("{} is not a regular file.", filepath));
                std::ifstream inputFile(filepath);
                if (!inputFile)
                        throw std::invalid_argument(fmt_ns::format("Failed opening {} for reading.", filepath));
                return inputFile; // NVRO
        }

        void create_export_directory(std::string_view dirname)
        {
                std::filesystem::create_directories(dirname);
        }

        void enter_export_directory(std::string_view dirname)
        {
                std::filesystem::current_path(dirname);
        }

        void exit_export_directory(auto original_path)
        {
                std::filesystem::current_path(original_path);
        }

} // namespace (Anonymous)

namespace Alpha
{
        Driver::Driver(const std::string &source_filepath, bool show_parser_trace)
            : source_filepath_(source_filepath), // Convert std::string to std::filesystem::path implicitly
              flex_buffer_(source_filepath),
              lexer_ctx_(source_filepath),
              lt_(flex_buffer_.size() - k_flex_eof_padding)
        {
                g_show_parser_trace = show_parser_trace;
        }

        void Driver::run_syntax_analyzer()
        {
                parser_retval_ = alpha_yyparse(lexer_ctx_, parse_ctx_, st_, et_, lt_);
        }

        void Driver::display_symbol_table()
        {
                std::cout << COLOR_ASCII_FG_BLUE;
                const auto &symbol_per_scope_vector = st_.symbols_per_scope();
                for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
                {
                        if (symbol_per_scope_vector[scope].empty())
                                continue;
                        std::cout << fmt_ns::format("----------------------------     Scope #{:<4}     ----------------------------\n", scope);
                        for (auto symbol_ptr : symbol_per_scope_vector[scope])
                                std::cout << fmt_ns::format(
                                    "{:<30} {:<20} (line {:>5}) (scope {:>4})\n",
                                    fmt_ns::format("\"{}\"", symbol_ptr->name),
                                    fmt_ns::format("[{}]", symbol_ptr->type_to_string()),
                                    lt_.find_symbol_line(symbol_ptr->location),
                                    symbol_ptr->scope);
                        std::cout << std::endl;
                }
                std::cout << SGR_RESET << std::endl;
        }

        void Driver::display_compile_errors() const
        {
                const std::string source_filename = source_filepath_.filename().string();

                for (const auto &error : et_.get_compile_time_errors())
                        std::cerr << error->make_pretty_diagnostic(
                            source_filename,
                            lt_,
                            flex_buffer_.const_buffer());
        }

        void Driver::export_symbol_table()
        {
                export_within_dir(k_symbol_table_exports_dirname, &Driver::export_symbol_table_impl);
        }

        void Driver::export_compile_errors()
        {
                export_within_dir(k_compile_error_exports_dirname, &Driver::export_compile_errors_impl);
        }

        Driver::FlexBuffer::FlexBuffer(const std::string &input_filepath)
        {
                std::ifstream input_file(open_alpha_source_file(input_filepath));
                const std::size_t input_file_size = std::filesystem::file_size(input_filepath);

                const std::size_t scan_buffer_size = input_file_size + k_flex_eof_padding;
                buffer_ = std::make_unique<char[]>(scan_buffer_size);

                if (!input_file.read(buffer_.get(), input_file_size))
                        throw std::runtime_error("Failed reading input_file.");

                // Flex requires two NULL-bytes at the end of the buffer (End-Of-Buffer marker).
                buffer_[input_file_size] = buffer_[input_file_size + 1] = '\0';
                size_ = scan_buffer_size;
                state_ = alpha_yy_scan_buffer(buffer_.get(), size_);
                if (state_ == nullptr)
                        throw std::runtime_error(
                            "Failed to initialize Flex scanner for file '" + input_filepath +
                            "': alpha_yy_scan_buffer returned nullptr (buffer size=" +
                            std::to_string(size_) + ").");
        }

        Driver::FlexBuffer::~FlexBuffer()
        {
                // Class invariant: `state_` must be non-null after construction.
                // Violations indicate a serious logic error (e.g., double-deletion or moved-from object).
                SMART_ASSERT(state_ != nullptr);
                state_ = nullptr; 
                // We nullified our reference to YY_BUFFER_STATE
                // Flex has it own, also we DID NOT use alpha_yy_delete_buffer()
                // So we can call alpha_yylex_destroy() on driver, which call delete_buffer()
                // and also deletes its own stack.
        }

        void Driver::export_symbol_table_impl()
        {
                const std::string outfile_name = source_filepath_.filename().string() + ".st.csv";
                std::ofstream outfile(outfile_name);
                if (!outfile)
                        throw std::runtime_error(fmt_ns::format(
                            "Failed opening file {} to export symbol table", outfile_name));

                outfile << k_symbol_table_csv_export_header; // Write CSV header.

                auto write_symbol = [&](const Symbol *symbol_ptr)
                {
                        outfile << fmt_ns::format(
                            "{},{},{},{}\n",
                            symbol_ptr->name,
                            symbol_ptr->type_to_string(),
                            lt_.find_symbol_line(symbol_ptr->location),
                            symbol_ptr->scope);
                };

                const auto &symbol_per_scope_vector = st_.symbols_per_scope();
                for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
                        for (const Symbol *symbol_ptr : symbol_per_scope_vector[scope])
                                write_symbol(symbol_ptr);
        }

        void Driver::export_within_dir(std::string_view dirname, void (Driver::*export_func)())
        {
                const auto original_path = std::filesystem::current_path();
                create_export_directory(dirname);
                enter_export_directory(dirname);
                (this->*export_func)();
                exit_export_directory(original_path);
        }

        void Driver::export_compile_errors_impl()
        {
                const std::string outfile_name = source_filepath_.filename().string() + ".error.csv";
                std::ofstream outfile(outfile_name);
                if (!outfile)
                        throw std::runtime_error(fmt_ns::format(
                            "Failed opening file {} to export compile errors", outfile_name));

                outfile << k_error_csv_export_header; // Write CSV header.
                auto write_diagnostic = [&](const Diagnostic &diag)
                {
                        outfile << fmt_ns::format(
                            "{},{},{},{}\n",
                            diag.line(lt_),
                            diag.column(lt_),
                            diag.type_to_string(),
                            diag.message);
                };

                for (const auto &cte : et_.get_compile_time_errors())
                {
                        write_diagnostic(cte->error);
                        for (const Diagnostic &note : cte->note_list)
                                write_diagnostic(note);
                }
        }
}