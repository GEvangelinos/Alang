#ifndef TRANSLATION_UNIT_HPP
#define TRANSLATION_UNIT_HPP
#include <filesystem>
#include <memory>
#include <scanner/scanner_context.hpp>

#include "compilation_options.hpp"
#include "core/basics.hpp"
#include "L1_driver/semantic_system.hpp"

// Forward declaration instead of including the <parser/alpha_parser.gen.hpp> header
typedef void *yyscan_t;

namespace alpha
{
class LocationTracker;
class DiagnosticReporter;
class SymbolTable;

class TUBuffer
{
public:
    const std::size_t null_padding;

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

class PassManager : private Immobile
{
public:
    PassManager(
        TUBuffer &tu_buffer,
        LocationTracker &lt,
        DiagnosticEngine &diagnostic_engine,
        SymbolTable *symbol_table);

    void execute();
    void notify_hard_error();

    [[nodiscard]] bool is_in_hard_error();
    [[nodiscard]] auto get_quads() { return semantic_system_.status_gateway.get_quads(); }

private:
    enum class Phase { FRONTEND };

    class ScannerHandle : private alpha::Immobile
    {
    public:
        ScannerHandle() = delete;
        explicit ScannerHandle(TUBuffer &tu_buffer);
        ~ScannerHandle();

        [[nodiscard]] yyscan_t get() const noexcept { return scanner_; }

    private:
        yyscan_t scanner_;
    };

    Phase running_phase_ = Phase::FRONTEND;
    SemanticSystem::Options ss_options_ = {false, false, false, false};

    LocationTracker &lt_;
    DiagnosticEngine &diagnostic_engine_;
    ScannerHandle scanner_handle_;
    ParseCtx parse_ctx_;
    LexerCtx lexer_ctx_;
    SemanticSystem semantic_system_;

    Once<int> parser_retval_;

    void run_frontend();
};

class TranslationUnit
{
public:
    TranslationUnit(
        const std::filesystem::path &source_path, CompilationOptions::Values comp_options);

    ~TranslationUnit() = default;

    void compile();
    void show_symbol_table() const;
    void show_compile_issues() const;
    void show_quads() const;
    void export_symbol_table() const;
    void export_compile_errors() const;
    void export_quads() const;
    [[nodiscard]] bool compiled_ok() { return compiled_ok_; }

private:
    const std::filesystem::path source_path_;
    const CompilationOptions::Values compilation_options_;
    TUBuffer tu_buffer_;
    LocationTracker loc_tracker_;
    DiagnosticEngine diagnostic_engine_;
    SymbolTable symbol_table_;
    std::unique_ptr<PassManager> pass_manager_;
    bool compiled_ok_ = false;

    void export_within_dir(
        std::string_view dirname, void (TranslationUnit::*export_func)() const) const;
    void export_symbol_table_impl() const;
    void export_compile_errors_impl() const;
    void export_quads_impl() const;
    [[nodiscard]] DiagnosticEngine::Policy create_diagnostic_engine_policy();

    // Notifiers (used as callbacks)
    static void notify_fatal_error();
    static void notify_max_errors_reached();
};
} // namespace alpha
#endif // TRANSLATION_UNIT_HPP
