#ifndef ALPHA_DRIVER_HPP
#define ALPHA_DRIVER_HPP

#include "translation_unit.hpp"
#include "settings/compiler_settings.hpp"

namespace alpha
{
class Driver : private Immobile
{
public:
    explicit Driver(const SettingManager& setting_manager);

    ~Driver() = default;

    // Simple wrappers for now
    void only_lex_tokens() const;
    void show_symbol_table() const;
    void show_symbol_table_without_temps() const;
    void show_diagnostics() const;
    void show_ir(bool detailed) const;
    void show_abc() const;
    void export_symbol_table() const;
    void export_symbol_table_without_temps() const;
    void export_diagnostics() const;
    void export_ir() const;
    void emit_abc() const;

    void run();

    bool ok() const noexcept;

private:
    std::list<TranslationUnit> translation_units_;
};
} // namespace alpha
#endif // ALPHA_DRIVER_HPP
