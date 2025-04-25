#ifndef ALPHA_ERROR_HPP
#define ALPHA_ERROR_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <list>
#include "core/alpha_location.hpp"
#include <string_view>

namespace Alpha
{
        // Classes defined here:
        class Diagnostic;
        class CTError;
        class ErrorTracker;

        class Diagnostic
        {
        public:
                enum class Type
                {
                        ERROR,
                        NOTE,
                };

                const Type type;
                const std::string message;
                const Location location;

                Diagnostic(Type type, const std::string &message, Location location);

                u32 line(const LocationTracker &lt) const;
                u32 column(const LocationTracker &lt) const;
                std::string_view type_to_string() const noexcept;
                std::string_view pretty_color() const noexcept;
        };

        class CTError
        {
        public:
                enum class Type
                {
                        LEXER,
                        SEMANTIC,
                        SYNTAX,
                };

                const Type type;
                const Diagnostic error;
                const std::list<Diagnostic> note_list;

                CTError() = delete;

                std::string_view type_to_string() const noexcept;
                std::string make_pretty_diagnostic(const std::string &source_filename,
                                                   const LocationTracker &lt,
                                                   const char *input_buffer) const;

        private:
                CTError(Type error_type, const std::string &error, Location error_location);
                CTError(Type error_type, const std::string &error, Location error_location,
                        const std::string &note, Location note_location);
                CTError(Type error_type, const std::string &error, Location error_location,
                        std::list<Diagnostic> &&note_list_);

                std::string make_pretty_diagnostic_impl(
                    const std::string &source_filename,
                    const LocationTracker &lt,
                    const char *input_buffer,
                    const Diagnostic &diagnostic) const;

                friend class ErrorTracker;
        };

        class ErrorTracker
        {
        public:
                ErrorTracker() = default;
                ErrorTracker(const ErrorTracker &) = delete;
                ErrorTracker(const ErrorTracker &&) = delete;
                ErrorTracker &operator=(const ErrorTracker &) = delete;
                ErrorTracker &operator=(ErrorTracker &&) = delete;

                void report_error(CTError::Type error_type, const std::string &error, Location error_location);
                void report_error(CTError::Type error_type, const std::string &error, Location error_location,
                                  const std::string &note, Location note_location);
                void report_error(CTError::Type errro_type, const std::string &error, Location error_location,
                                  std::list<Diagnostic> &&note_list_);

                const std::vector<const CTError *> &get_compile_time_errors() const noexcept { return cterrors_; }

        private:
                std::vector<const CTError *> cterrors_;
        };
} // namespace Alpha
#endif // ALPHA_ERROR_HPP
