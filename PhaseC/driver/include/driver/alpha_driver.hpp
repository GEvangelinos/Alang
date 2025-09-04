#include <filesystem>                      // for path
#include <string>                          // for string
#include "compilation_options.hpp"
#include "translation_unit.hpp"
#include "diagnostics/diagnostic_engine.hpp"

namespace alpha
{

class Driver : private Immobile
{
public:
    explicit Driver(const std::string &source_filepath, CompilationOptions::Values comp_options)
        : tu(std::filesystem::path(source_filepath), std::move(comp_options)) {}

    ~Driver() = default;

    // Simple wrappers for now
    void show_symbol_table() const;
    void show_compile_issues() const;
    void show_ir() const;
    void export_symbol_table() const;
    void export_symbol_table_without_temps() const;
    void export_compile_errors() const;
    void export_ir() const;

    void run();

    bool ok() const;

private:
    // So far its only 1 TU, I made parsing system reentrant, to let multiple tu get compiled at once.
    TranslationUnit tu;
};
} // namespace alpha
