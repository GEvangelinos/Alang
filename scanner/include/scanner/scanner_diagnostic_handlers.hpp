#ifndef ALPHA_SCANNER_DIAGNOSTIC_HANDLERS_HPP
#define ALPHA_SCANNER_DIAGNOSTIC_HANDLERS_HPP

#include "core/machine_types.hpp"
#include "core/numeric_types.hpp"

namespace alpha
{
class SourceLocation;
class DiagnosticReporter;


namespace scanner_diagnostic_handlers
{
    [[nodiscard]] AlphaInt handle_invalid_integer_suffix(
        DiagnosticReporter &dr,
        const char *text,
        u64 text_len,
        const SourceLocation &full_literal_loc
    );
} // namespace scanner_diagnostic_handlers
} // namespace alpha

#endif // ALPHA_SCANNER_DIAGNOSTIC_HANDLERS_HPP
