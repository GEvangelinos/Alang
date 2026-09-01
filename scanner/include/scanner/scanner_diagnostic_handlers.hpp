#ifndef ALPHA_SCANNER_DIAGNOSTIC_HANDLERS_HPP
#define ALPHA_SCANNER_DIAGNOSTIC_HANDLERS_HPP

#include <string_view>
#include "core/machine_types.hpp"

namespace alpha
{
class SourceLocation;
class DiagnosticReporter;

namespace scanner_diagnostic_handlers
{
    [[nodiscard]] AlphaInt handle_invalid_integer_suffix(
        DiagnosticReporter &dr,
        std::string_view lexeme,
        SourceLocation lexeme_loc
    );

    [[nodiscard]] AlphaFloat handle_invalid_float_suffix(
        DiagnosticReporter &dr,
        std::string_view lexeme,
        SourceLocation lexeme_loc
    );

    [[nodiscard]] AlphaFloat handle_missing_exponent_digits(
        DiagnosticReporter &dr,
        std::string_view lexeme,
        SourceLocation lexeme_loc
    );
} // namespace scanner_diagnostic_handlers
} // namespace alpha

#endif // ALPHA_SCANNER_DIAGNOSTIC_HANDLERS_HPP
