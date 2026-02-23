#ifndef TRANSLATION_UNIT_HPP
#define TRANSLATION_UNIT_HPP
#include <filesystem>
#include <memory>
#include <scanner/scanner_context.hpp>

#include "core/basics.hpp"
#include "diagnostics/diagnostic_formatter.hpp"
#include "L1_driver/semantic_system.hpp"
#include "settings/compiler_settings.hpp"

// Forward declaration instead of including the <parser/alpha_parser.gen.hpp> header
typedef void *yyscan_t;

namespace alpha
{
class IROptimizer;
class LocationTracker;
class DiagnosticReporter;
class SymbolTable;
class ScannerAdapter;

class PassManager : private Immobile
{
public:
    PassManager(
        const settings::ExprOpts &expr_opts,
        const settings::IROpts &ir_opts,
        TranslationUnitBuffer &tu_buffer,
        LocationTracker &lt,
        DiagnosticEngine &diagnostic_engine,
        SymbolTable *symbol_table);

    void execute();
    void notify_hard_error();

    [[nodiscard]] bool is_in_hard_error()const ;
    [[nodiscard]] const std::vector<Quad> &get_quads() const noexcept;

private:
    enum class Phase { FRONTEND, IR_OPTIMIZATION };

    Phase running_phase_ = Phase::FRONTEND;

    LocationTracker &lt_;
    DiagnosticEngine &diagnostic_engine_;
    ParseCtx parse_ctx_;
    LexerCtx lexer_ctx_;
    std::unique_ptr<ScannerAdapter> scanner_;
    SemanticSystem semantic_system_;
    std::unique_ptr<IROptimizer> ir_optimizer_;
    std::vector<Quad> ir_quads_;

    Once<int> parser_retval_;

    void run_frontend();
};

class TranslationUnit
{
public:
    TranslationUnit(
        const std::filesystem::path &source_path,
        std::size_t max_errors,
        const alpha::settings::ExprOpts &expr_opts,
        const alpha::settings::IROpts &ir_opts);

    ~TranslationUnit();

    void compile();
    void show_symbol_table() const;
    void show_diagnostics() const;
    void show_ir(bool detailed) const;
    void export_symbol_table() const;
    void export_symbol_table_without_temps() const;
    void export_diagnostics() const;
    void export_ir() const;

    [[nodiscard]] bool compiled_ok() const noexcept;

private:
    const std::filesystem::path source_path_;
    const alpha::settings::ExprOpts expr_opts_;
    DiagnosticEngine diagnostic_engine_;
    // Using a pointer (unique_ptr) to guard against wrong initialization order
    std::unique_ptr<TranslationUnitBuffer> translation_unit_buffer_;
    LocationTracker loc_tracker_;
    DiagnosticFormatter diagnostic_formatter_;
    SymbolTable symbol_table_;
    std::unique_ptr<PassManager> pass_manager_;
    OnceFlag tried_compiling;
    bool execution_completed_ = false;

    void export_within_dir(
        std::string_view dirname, void (TranslationUnit::*export_func)() const) const;
    void export_symbol_table_dispatch() const;
    void export_symbol_table_without_temps_dispatch() const;
    void export_symbol_table_impl(bool export_temps) const;
    void export_diagnostics_impl() const;
    void export_ir_impl() const;
    [[nodiscard]] DiagnosticEngine::Policy create_diagnostic_engine_policy();

    // Notifiers (used as callbacks)
    static void notify_fatal_error();
    static void notify_max_errors_reached();
};
} // namespace alpha
#endif // TRANSLATION_UNIT_HPP
