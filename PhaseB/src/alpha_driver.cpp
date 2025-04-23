#include "alpha_driver.hpp"

static constexpr unsigned k_flex_eof_padding = 2;

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

        void create_export_directory(const std::string &dirname)
        {
                std::filesystem::create_directories(dirname);
        }

        void enter_export_directory(const std::string &dirname)
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
                alpha_yy_delete_buffer(state_);
        }

        Driver::Driver(const std::string source_filepath)
            : source_filepath_(source_filepath), // Convert std::string to std::filesystem::path implicitly
              flex_buffer_(source_filepath),
              lexer_ctx_(source_filepath),
              lt_(flex_buffer_.size() - k_flex_eof_padding)
        {
        }

        bool Driver::ok()
        {
                return ok_flag_;
        }

        void Driver::run_syntax_analyzer()
        {
                parser_retval_ = alpha_yyparse(lexer_ctx_, parse_ctx_, st_, et_, lt_);
        }

        void Driver::display_symbol_table()
        {
                std::cout << COLOR_ASCII_FG_BLUE;
                const auto &symbol_per_scope_vector = st_.symbols_per_scope();
                for (Alpha::u32 scope = Alpha::k_global_scope;
                     scope < symbol_per_scope_vector.size();
                     scope++)
                {
                        if (symbol_per_scope_vector[scope].empty())
                                continue;
                        std::cout << fmt_ns::format("----------------------------     Scope #{:<4}     ----------------------------\n", scope);
                        for (auto symbol_ptr : symbol_per_scope_vector[scope])
                                std::cout << fmt_ns::format(
                                    "{:<30} {:<20} (line {:>5}) (scope {:>4})\n",
                                    fmt_ns::format("\"{}\"", symbol_ptr->name()),
                                    fmt_ns::format("[{}]", Alpha::to_string(symbol_ptr->type())),
                                    lt_.find_symbol_line(symbol_ptr->location()),
                                    symbol_ptr->scope());
                        std::cout << std::endl;
                }
                std::cout << SGR_RESET << std::endl;
        }

        void Driver::display_compilation_errors()
        {
                for (const CompileTimeError *error : et_.ger_error_vector())
                        std::cerr << error->to_string() << std::endl;
        }

        void Driver::export_symbol_table(std::optional<std::string> exports_dirname)
        {
                const auto original_path = std::filesystem::current_path();
                create_export_directory(exports_dirname.value_or(k_default_exports_dirname));
                enter_export_directory(exports_dirname.value_or(k_default_exports_dirname));
                export_symbol_table_impl();
                exit_export_directory(original_path);
        }

        void Driver::export_symbol_table_impl()
        {

                const std::string outfile_name = source_filepath_.filename().string() + ".csv";
                std::ofstream outfile(outfile_name);
                if (!outfile)
                        throw std::runtime_error(fmt_ns::format(
                            "Failed opening {} for writing symtable_export", outfile_name));

                outfile << k_symtable_csv_export_header; // Write CSV header.

                const auto &symbol_per_scope_vector = st_.symbols_per_scope();
                for (Alpha::u32 scope = Alpha::k_global_scope;
                     scope < symbol_per_scope_vector.size();
                     scope++)
                {
                        for (auto symbol_ptr : symbol_per_scope_vector[scope])
                                outfile << fmt_ns::format(
                                    "{},{},{},{}\n",
                                    symbol_ptr->name(),
                                    Alpha::to_string(symbol_ptr->type()),
                                    lt_.find_symbol_line(symbol_ptr->location()),
                                    symbol_ptr->scope());
                }
        }

}