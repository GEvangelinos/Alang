#include <cstddef>                         // for size_t
#include <filesystem>                      // for path
#include <memory>                          // for unique_ptr
#include <string>                          // for string
#include <string_view>                     // for string_view
#include <fstream>
#include "core/source_location.hpp"         // for LocationTracker
#include "diagnostics/diagnostic_engine.hpp"
#include "parser/parser_context.hpp"
#include "parser/symbol_table.hpp"     // for SymbolTable
#include "parser/L1_driver/semantic_system.hpp"
#include "scanner/scanner_context.hpp" // for LexerCtx

typedef void* yyscan_t; // Forward declaration instead of including the <parser/alpha_parser.gen.hpp> header

namespace alpha
{
static constexpr char k_symbol_table_exports_dirname[] = "SYMBOL_TABLE_EXPORTS";
static constexpr char k_compile_error_exports_dirname[] = "COMPILE_ERROR_EXPORTS";
static constexpr char k_quad_exports_dirname[] = "QUAD_EXPORTS";
static constexpr char k_symbol_table_csv_export_header[] = "symbol,type,line,scope\n";
static constexpr char k_error_csv_export_header[] = "line,column,diagnostic_type,message\n";

class Driver : private Immobile
{
public:
    Driver(const std::string &source_filepath, bool show_parser_trace);
    ~Driver();

    void run();
    void show_symbol_table() const;
    void show_compile_issues() const;
    void show_quads() const;
    void export_symbol_table() const;
    void export_compile_errors() const;
    void export_quads() const;
    [[nodiscard]] bool ok() const noexcept { return ok_flag_; }

    static void notify_fatal_error();

private:
    class CompilationPipeline; // FWD

    const std::filesystem::path source_filepath_;
    LocationTracker lt_;
    DiagnosticEngine diagnostic_engine_;
    SymbolTable st_;
    std::unique_ptr<CompilationPipeline> compilation_pipeline;

    bool ok_flag_ = true;

    void export_within_dir(std::string_view dirname, void (Driver::*export_func)() const) const;
    void export_symbol_table_impl() const;
    void export_compile_errors_impl() const;
    void export_quads_impl() const;
    [[nodiscard]] DiagnosticEngine::Policy create_diagnostic_engine_policy();
};

class Driver::CompilationPipeline : private Immobile
{
public:
    CompilationPipeline(
        const std::filesystem::path &source_filepath,
        LocationTracker &lt,
        DiagnosticReporter &dr,
        SymbolTable *symbol_table);

    void compile();
    void notify_hard_error();

    [[nodiscard]] bool is_in_hard_error();
    [[nodiscard]] auto get_quads() { return semantic_system_.status_gateway.get_quads(); }

private:
    enum class Phase { FRONTEND };

    Phase running_phase_ = Phase::FRONTEND;

    SemanticSystem::Options ss_options_ = {false, false, false, false};

    LocationTracker &lt_;
    DiagnosticReporter &dr_;
    LexerCtx lexer_ctx_;
    ParseCtx parse_ctx_;
    SemanticSystem semantic_system_;

    Once<int> parser_retval_;

    void run_frontend();
};

class TUBuffer
{
public:
    explicit TUBuffer(const std::filesystem::path &path, std::size_t null_padding);
    ~TUBuffer() = default;

    [[nodiscard]] char *data() { return data_.get(); }
    [[nodiscard]] const char *data() const { return data_.get(); }
    [[nodiscard]] std::size_t size() const { return size_; }

private:
    std::unique_ptr<char[]> data_;
    std::size_t size_ = 0;

    [[nodiscard]] static std::ifstream open_source(const std::filesystem::path &path);
};

class TranslationUnit
{
    constexpr auto k_scanner_eof_null_padding = 2; // For 2 consecutive NULL bytes.
public:
    explicit TranslationUnit(const std::filesystem::path &source_path);
    ~TranslationUnit() = default;

private:
    class ScannerHandle : private Immobile
    {
    public:
        ScannerHandle() = delete;
        explicit ScannerHandle(TUBuffer &tu_buffer);
        ~ScannerHandle();

        yyscan_t get() const noexcept { return scanner_; }

    private:
        yyscan_t scanner_;
    };

    const std::filesystem::path source_path_;
    TUBuffer tu_buffer_;
    ScannerHandle scanner_handle_;
};
} // namespace alpha
