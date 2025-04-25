#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include "utils/format_adapter.hpp"
#include "arguinator/arguinator.hpp"
#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_location.hpp"
#include "scanner/alpha_scanner_context.hpp"
#include "alpha_scanner.hpp"
#include "core/alpha_konstants.hpp"
#include "alpha_parser.hpp"
#include "core/alpha_error.hpp"
#include "core/alpha_types.hpp"
#include "utils/cli_color.h"
#include <optional>
namespace Alpha
{
        // Classes defined here:
        class Driver;
        // class Driver::FlexBuffer;

        static constexpr char k_default_symtable_exports_dirname[] = "SYMBOL_TABLE_EXPORTS";
        static constexpr char k_default_error_exports_dirname[] = "ERROR_EXPORTS";
        static constexpr char k_symbol_table_csv_export_header[] = "symbol,type,line,scope\n";
        static constexpr char k_error_csv_export_header[] = "line,column,diagnostic_type,message\n";

        class Driver
        {
        public:
                explicit Driver(const std::string &source_filepath);
                ~Driver() = default;
                Driver(const Driver &) = delete;
                Driver(const Driver &&) = delete;
                Driver &operator=(const Driver &) = delete;
                Driver &operator=(Driver &&) = delete;

                void run_syntax_analyzer();
                void display_symbol_table();
                void display_compilation_errors() const;
                void export_symbol_table(std::optional<std::string> dirname);
                void export_compilation_errors(std::optional<std::string> dirname);
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

                void export_symbol_table_impl();
                void export_compilation_errors_impl();
        };
}