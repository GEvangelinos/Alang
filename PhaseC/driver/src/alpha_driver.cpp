#include "driver/alpha_driver.hpp"

namespace alpha
{
void
Driver::show_symbol_table() const
{
    if (this->ok())
        tu.show_symbol_table();
}

void
Driver::show_compile_issues() const { tu.show_compile_issues(); }

void
Driver::show_ir() const
{
    if (this->ok())
        tu.show_ir();
}

void
Driver::export_symbol_table() const
{
    if (this->ok())
        tu.export_symbol_table();
}

void
Driver::export_symbol_table_without_temps() const
{
    if (this->ok())
        tu.export_symbol_table_without_temps();
}

void
Driver::export_compile_errors() const { tu.export_compile_errors(); }

void
Driver::export_ir() const
{
    if (this->ok())
        tu.export_ir();
}

void
Driver::run() { tu.compile(); }

bool
Driver::ok() const { return tu.compiled_ok(); }
} // namespace alpha
