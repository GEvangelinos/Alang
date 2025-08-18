#ifndef TRANSLATION_UNIT_HPP
#define TRANSLATION_UNIT_HPP
#include <filesystem>
#include <memory>
#include <scanner/scanner_context.hpp>
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
        DiagnosticReporter &dr,
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

        yyscan_t get() const noexcept { return scanner_; }

    private:
        yyscan_t scanner_;
    };

    Phase running_phase_ = Phase::FRONTEND;
    SemanticSystem::Options ss_options_ = {false, false, false, false};

    LocationTracker &lt_;
    DiagnosticReporter &dr_;
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
    explicit TranslationUnit(const std::filesystem::path &source_path);
    ~TranslationUnit() = default;

    void compile();
    void show_symbol_table() const;
    void show_compile_issues() const;
    void show_quads() const;
    void export_symbol_table() const;
    void export_compile_errors() const;
    void export_quads() const;
    bool compiled_ok() { return compiled_ok_; }

private:
    const std::filesystem::path source_filepath_;

    void export_within_dir(
        std::string_view dirname, void (TranslationUnit::*export_func)() const) const;
    void export_symbol_table_impl() const;
    void export_compile_errors_impl() const;
    void export_quads_impl() const;

    static void notify_fatal_error();

private:
    const std::filesystem::path source_path_;
    TUBuffer tu_buffer_;
    LocationTracker lt_;
    DiagnosticEngine diagnostic_engine_;
    SymbolTable st_;
    std::unique_ptr<PassManager> compilation_pipeline_;

    bool compiled_ok_ = false;

    [[nodiscard]] DiagnosticEngine::Policy create_diagnostic_engine_policy();
};
} // namespace alpha
#endif // TRANSLATION_UNIT_HPP
