#include <cstddef>                           // for size_t
#include <filesystem>                        // for path
#include <memory>                            // for unique_ptr
#include <string>                            // for string
#include <string_view>                       // for string_view
#include "alpha_scanner.hpp"                 // for YY_BUFFER_STATE
#include "core/alpha_error.hpp"              // for ErrorTracker
#include "core/alpha_location.hpp"           // for LocationTracker
#include "parser/alpha_parser_context.hpp"   // for ParseCtx
#include "parser/alpha_symbol_table.hpp"     // for SymbolTable
#include "scanner/alpha_scanner_context.hpp" // for LexerCtx

namespace Alpha
{
        // Classes defined here:
        class Driver; // IWYU pragma: keep
        // class Driver::FlexBuffer; // IWYU pragma: keep

        static constexpr char k_symtable_exports_dirname[] = "SYMBOL_TABLE_EXPORTS";
        static constexpr char k_error_exports_dirname[] = "ERROR_EXPORTS";
        static constexpr char k_symbol_table_csv_export_header[] = "symbol,type,line,scope\n";
        static constexpr char k_error_csv_export_header[] = "line,column,diagnostic_type,message\n";

        class Driver
        {
        public:
                explicit Driver(const std::string &source_filepath, bool show_parser_trace);
                ~Driver()
                {
                        alpha_yylex_destroy();
                }
                Driver(const Driver &) = delete;
                Driver(const Driver &&) = delete;
                Driver &operator=(const Driver &) = delete;
                Driver &operator=(Driver &&) = delete;

                void run_syntax_analyzer();
                void display_symbol_table();
                void display_compile_errors() const;
                void export_symbol_table();
                void export_compile_errors();
                bool ok() { return ok_flag_; }

        private:
                class FlexBuffer
                {
                public:
                        FlexBuffer(const std::string &input_filepath);
                        ~FlexBuffer();
                        FlexBuffer(const FlexBuffer &) = delete;
                        FlexBuffer(const FlexBuffer &&) = delete;
                        FlexBuffer &operator=(const FlexBuffer &) = delete;
                        FlexBuffer &operator=(FlexBuffer &&) = delete;

                        char *buffer() { return buffer_.get(); }
                        const char *const_buffer() const { return buffer_.get(); }
                        std::size_t size() const { return size_; }

                private:
                        std::unique_ptr<char[]> buffer_;
                        std::size_t size_;
                        YY_BUFFER_STATE state_ = nullptr;
                };

                const std::filesystem::path source_filepath_;
                FlexBuffer flex_buffer_;
                Alpha::LexerCtx lexer_ctx_;
                Alpha::ParseCtx parse_ctx_;
                Alpha::SymbolTable st_;
                Alpha::ErrorTracker et_;
                Alpha::LocationTracker lt_;
                int parser_retval_ = 0;
                bool ok_flag_ = true;

                void export_within_dir(std::string_view dirname, void (Driver::*export_func)());
                void export_symbol_table_impl();
                void export_compile_errors_impl();
        };
}