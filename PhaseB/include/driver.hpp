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
#include "core/alpha_error_tracker.hpp"
#include "core/alpha_types.hpp"
#include "utils/cli_color.h"
#include <optional>
namespace Alpha
{

        static constexpr char k_default_exports_dirname[] = "SYMTABLE_EXPORTS";
        static constexpr char k_symtable_csv_export_header[] = "symbol,type,line,scope\n";

        class Driver
        {
        public:
                explicit Driver(const std::string source_filepath);
                ~Driver() = default;
                Driver(const Driver &) = delete;
                Driver(const Driver &&) = delete;
                Driver &operator=(const Driver &) = delete;
                Driver &operator=(Driver &&) = delete;

                void run_syntax_analyzer();
                void display_symbol_table();
                void export_symbol_table(std::optional<std::string> exports_dirname);
                bool ok();

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
                        std::size_t size() { return size_; }

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
        };
}