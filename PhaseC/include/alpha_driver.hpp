#include <cstddef>                         // for size_t
#include <filesystem>                      // for path
#include <memory>                          // for unique_ptr
#include <string>                          // for string
#include <string_view>                     // for string_view
#include "alpha_scanner.hpp"               // for YY_BUFFER_STATE
#include "core/alpha_diagnostics.hpp"            // for CTIssueTracker
#include "core/alpha_location.hpp"         // for LocationTracker
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "parser/alpha_semantic_driver.hpp"
#include "parser/alpha_symbol_table.hpp"     // for SymbolTable
#include "scanner/alpha_scanner_context.hpp" // for LexerCtx

namespace Alpha
{
// Classes defined here:
class Driver; // IWYU pragma: keep
// class Driver::FlexBuffer; // IWYU pragma: keep

static constexpr char k_symbol_table_exports_dirname[] = "SYMBOL_TABLE_EXPORTS";
static constexpr char k_compile_error_exports_dirname[] = "COMPILE_ERROR_EXPORTS";
static constexpr char k_quad_exports_dirname[] = "QUAD_EXPORTS";
static constexpr char k_symbol_table_csv_export_header[] = "symbol,type,line,scope\n";
static constexpr char k_error_csv_export_header[] = "line,column,diagnostic_type,message\n";

class Driver : private Immobile
{
public:
        explicit Driver(const std::string &source_filepath, bool show_parser_trace);
        ~Driver() { alpha_yylex_destroy(); }

        void run_alpha_parser();
        void show_symbol_table() const;
        void show_compile_issues() const;
        void show_quads() const;
        void export_symbol_table() const;
        void export_compile_errors() const;
        void export_quads() const;
        [[nodiscard]] bool ok() const noexcept { return ok_flag_; }

private:
        class FlexBuffer : private Immobile
        {
        public:
                FlexBuffer(const std::string &input_filepath);
                ~FlexBuffer();

                [[nodiscard]] char *buffer() const { return buffer_.get(); }
                [[nodiscard]] const char *const_buffer() const { return buffer_.get(); }
                [[nodiscard]] std::size_t size() const { return size_; }

        private:
                std::unique_ptr<char[]> buffer_;
                std::size_t size_;
                YY_BUFFER_STATE state_ = nullptr;
        };

        const std::filesystem::path source_filepath_;
        FlexBuffer flex_buffer_;
        Alpha::LocationTracker lt_;
        Alpha::Diagnostics diagnostics_;
        Alpha::SymbolTable st_;
        Alpha::LexerCtx lexer_ctx_;
        Alpha::ParseCtx parse_ctx_;
        Alpha::SemanticDriver semantic_driver_;
        int parser_retval_ = 0;
        bool ok_flag_ = true;

        void export_within_dir(std::string_view dirname, void (Driver::*export_func)() const) const;
        void export_symbol_table_impl() const;
        void export_compile_errors_impl() const;
        void export_quads_impl() const;
};
} // namespace Alpha
