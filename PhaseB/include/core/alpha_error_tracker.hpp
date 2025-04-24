#ifndef ERROR_TRACKER_HPP
#define ERROR_TRACKER_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <list>
#include "core/alpha_location.hpp"
#include <string_view>
namespace Alpha
{

        class Diagnostic
        {
        public:
                enum class Type
                {
                        ERROR,
                        NOTE,
                };

                const std::string message;
                const Location location;
                const Type type;

                Diagnostic(const std::string &message, Location location, Type type);
                u32 line(const LocationTracker &lt) const;
                u32 column(const LocationTracker &lt) const;
                std::string type_to_string() const;
                std::string pretty_color() const;
        };

        class CompileTimeError
        {
        public:
                CompileTimeError() = delete;

                /* NOTE(2387091987120976)TODO: Do you really need both? Also Does the use need to know if it
                 * lexer of syntax error, in GCC you just know its an error, not which component
                 * triggered it, maybe add extra flag to show who triggered it, (custom argparse)!
                 * */
                virtual std::string get_error_type() const noexcept = 0;

                std::string make_pretty_diagnostic(
                    const std::string &source_filename,
                    const LocationTracker &lt,
                    const char *input_buffer) const;

        protected:
                Diagnostic error_;
                std::list<Diagnostic> note_list_;

                CompileTimeError(const std::string &error, Location error_location);

                CompileTimeError(const std::string &error, Location error_location,
                                 const std::string &note, Location note_location);

                CompileTimeError(const std::string &error, Location error_location,
                                 std::list<Diagnostic> &&note_list_);

        private:
                std::string make_pretty_diagnostic_impl(
                    const std::string &source_filename,
                    const LocationTracker &lt,
                    const char *input_buffer,
                    const Diagnostic &diagnostic) const;
        };

        class LexerError : public CompileTimeError
        {
        public:
                LexerError(const std::string &error, const Location error_location);

                LexerError() = delete;

                std::string get_error_type() const noexcept override { return "lexer"; }
        };

        class SyntaxError : public CompileTimeError
        {
        public:
                SyntaxError(const std::string &error, const Location error_location);

                SyntaxError(const std::string &error, const Location error_location,
                            const std::string &note, const Location note_location);

                SyntaxError(const std::string &error, const Location error_location,
                            std::list<Diagnostic> &&note_list_);

                SyntaxError() = delete;

                std::string get_error_type() const noexcept override { return "syntax"; }
        };

        class ErrorTracker
        {
        public:
                ErrorTracker() = default;

                void report_lexer_error(const std::string &error, Location error_location);

                void report_syntax_error(const std::string &error, Location error_location);

                void report_syntax_error(const std::string &error, Location error_location,
                                         const std::string &note, Location note_location);

                void report_syntax_error(const std::string &error, Location error_location,
                                         std::list<Diagnostic> &&note_list_);

                const std::vector<const CompileTimeError *> &ger_error_vector() const;

        private:
                std::vector<const CompileTimeError *> error_vector_;
        };
}

#endif /* ERROR_TRACKER_HPP */
