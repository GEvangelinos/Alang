#include "driver/alpha_driver.hpp"

namespace alpha
{
Driver::Driver(
    const alpha::settings::ConfigData &config_data,
    const alpha::settings::ExprOpts &expr_opts,
    const alpha::settings::IROpts &ir_opts)
{
    translation_units_.emplace_back(config_data.source, config_data.max_errors, expr_opts, ir_opts);
}

void
Driver::only_lex_tokens() const
{
    for (const TranslationUnit &tu : translation_units_)
        tu.only_lex_tokens();
}


void
Driver::show_symbol_table() const
{
    if (!this->ok())
        return;
    for (const TranslationUnit &tu : translation_units_)
        tu.show_symbol_table();
}

void
Driver::show_diagnostics() const
{
    for (const TranslationUnit &tu : translation_units_)
        tu.show_diagnostics();
}

void
Driver::show_ir(const bool detailed) const
{
    if (!this->ok())
        return;
    for (const TranslationUnit &tu : translation_units_)
        tu.show_ir(detailed);
}

void
Driver::export_symbol_table() const
{
    if (!this->ok())
        return;
    for (const TranslationUnit &tu : translation_units_)
        tu.export_symbol_table();
}

void
Driver::export_symbol_table_without_temps() const
{
    if (!this->ok())
        return;
    for (const TranslationUnit &tu : translation_units_)
        tu.export_symbol_table_without_temps();
}

void
Driver::export_diagnostics() const
{
    for (const TranslationUnit &tu : translation_units_)
        tu.export_diagnostics();
}

void
Driver::export_ir() const
{
    if (!this->ok())
        return;
    for (const TranslationUnit &tu : translation_units_)
        tu.export_ir();
}

void
Driver::run()
{
    for (TranslationUnit &tu : translation_units_)
        tu.compile();
}

bool
Driver::ok() const noexcept
{
    const bool all_tu_ok = std::all_of(
        translation_units_.begin(),
        translation_units_.end(),
        [](const TranslationUnit &tu) { return tu.compiled_ok(); }
    );
    return all_tu_ok;
}
} // namespace alpha
