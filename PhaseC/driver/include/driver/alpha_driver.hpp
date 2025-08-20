#include <filesystem>                      // for path
#include <string>                          // for string
#include "compilation_options.hpp"
#include "translation_unit.hpp"
#include "diagnostics/diagnostic_engine.hpp"

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
    explicit Driver(const std::string &source_filepath, CompilationOptions::Values comp_options)
        : tu(std::filesystem::path(source_filepath), std::move(comp_options)) {}

    ~Driver() = default;

    // Simple wrappers for now
    void show_symbol_table() const { tu.show_symbol_table(); }
    void show_compile_issues() const { tu.show_compile_issues(); }
    void show_quads() const { tu.show_quads(); }
    void export_symbol_table() const { tu.export_symbol_table(); }
    void export_compile_errors() const { tu.export_compile_errors(); }
    void export_quads() const { tu.export_quads(); }

    void run() { tu.compile(); }

    bool ok() { return tu.compiled_ok(); }

private:
    // So far its only 1 TU, I made parsing system reentrant, to let multiple tu get compiled at once.
    TranslationUnit tu;
};
} // namespace alpha
