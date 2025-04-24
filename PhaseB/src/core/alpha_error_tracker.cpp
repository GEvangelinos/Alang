#include "utils/format_adapter.hpp"
#include "core/alpha_error_tracker.hpp"

#include <sstream>
#include "utils/cli_color.h"
#include "utils/format_adapter.hpp"
#include "core/alpha_location.hpp"
#include <iostream>
#include <iomanip>
#include "core/alpha_macros.hpp"

namespace // (Anonymous)
{
        std::vector<std::string_view> extract_line_views(const char *buffer, std::size_t start_index, std::size_t end_index)
        {
                std::vector<std::string_view> lines;
                const char *start = buffer + start_index;
                const char *current = start;
                const char *end_target = buffer + end_index;

                while (*current)
                {
                        const char *line_end = std::strchr(current, '\n');
                        if (!line_end)
                                line_end = current + std::strlen(current); // to '\0' if no newline

                        lines.emplace_back(current, line_end - current); // view [current, line_end)

                        if (end_target <= line_end)
                                break; // stop if end_index is in this line

                        current = (*line_end == '\n') ? line_end + 1 : line_end;
                }
                return lines;
        }
}

namespace Alpha
{
        Diagnostic::Diagnostic(const std::string &message, Location location, Diagnostic::Type type)
            : message(message), location(location), type(type) {}

        CompileTimeError::CompileTimeError(const std::string &error_message, Location error_location)
            : error_(error_message, error_location, Diagnostic::Type::ERROR) {}

        CompileTimeError::CompileTimeError(const std::string &error_message, Location error_location,
                                           const std::string &note_message, Location note_location)
            : error_(error_message, error_location, Diagnostic::Type::ERROR),
              note_list_{{note_message, note_location, Diagnostic::Type::NOTE}} {}

        CompileTimeError::CompileTimeError(const std::string &error_message, Location error_location,
                                           std::list<Diagnostic> &&note_list_)
            : error_(error_message, error_location, Diagnostic::Type::ERROR),
              note_list_(std::move(note_list_)) {}

        u32 Diagnostic::line(const LocationTracker &lt) const
        {
                return lt.find_first_line(location);
        }

        u32 Diagnostic::column(const LocationTracker &lt) const
        {
                return lt.find_first_column(location);
        }

        std::string Diagnostic::type_to_string() const
        {
                switch (type)
                {
                case Type::ERROR:
                        return "error";
                case Type::NOTE:
                        return "note";
                }
                UNREACHABLE("Diagnostic type was not registered, please register it");
        }

        std::string Diagnostic::pretty_color() const
        {
                switch (type)
                {
                case Type::ERROR:
                        return COLOR_ASCII_BOLD_RED;
                case Type::NOTE:
                        return COLOR_ASCII_BOLD_GREEN;
                }
                UNREACHABLE("Diagnostic type was not registered, please register it");
        }

        std::string CompileTimeError::make_pretty_diagnostic_impl(
            const std::string &source_filename,
            const LocationTracker &lt,
            const char *input_buffer,
            const Diagnostic &diagnostic) const
        {
                constexpr u32 line_box_width = 8;
                std::stringstream ss;

                /* Error header: */
                const u32 diagnostic_line = diagnostic.line(lt);
                const u32 diagnostic_column = diagnostic.column(lt);
                ss << fmt_ns::format(
                    "{}{}:{}:{}: {}{}{}: {}\n",
                    COLOR_ASCII_BOLD_DEFAULT, source_filename, diagnostic_line, diagnostic_column,
                    diagnostic.pretty_color(), diagnostic.type_to_string(), COLOR_ASCII_BOLD_DEFAULT,
                    diagnostic.message);

                ss << SGR_RESET;

                auto line_views = extract_line_views(
                    input_buffer, lt.find_index_of_line(diagnostic_line), diagnostic.location.last_index_);

                for (std::size_t i = 0; i < line_views.size(); i++)
                {
                        ss << fmt_ns::format("{:>{}} | {}\n",
                                             i != 0 ? "" : std::to_string(diagnostic_line),
                                             line_box_width,
                                             line_views[i]);
                        if (i != 0) // Caret marking is only for first line.
                                continue;
                        ss << fmt_ns::format("{:>{}} | {:{}}{}^{}\n",
                                             "", // Empty just a placeholder for spaces
                                             line_box_width,
                                             "", // Placeholder for spaces too.
                                             diagnostic.location.first_index_ - lt.find_index_of_line(lt.find_first_line(diagnostic.location)),
                                             diagnostic.pretty_color(),
                                             std::string(diagnostic.location.last_index_ - diagnostic.location.first_index_-1, '~'));
                        ss << SGR_RESET;
                }

                return ss.str();
        }

        std::string CompileTimeError::make_pretty_diagnostic(
            const std::string &source_filename,
            const LocationTracker &lt,
            const char *input_buffer) const
        {
                std::stringstream ss;
                ss << make_pretty_diagnostic_impl(source_filename, lt, input_buffer, error_);

                for (auto note : note_list_)
                        ss << make_pretty_diagnostic_impl(source_filename, lt, input_buffer, note);

                return ss.str();
        }

        LexerError::LexerError(const std::string &error_message, Location error_location)
            : CompileTimeError(error_message, error_location) {}

        SyntaxError::SyntaxError(const std::string &error_message, Location error_location)
            : CompileTimeError(error_message, error_location) {}

        SyntaxError::SyntaxError(const std::string &error, const Location error_location,
                                 const std::string &note, const Location note_location)
            : CompileTimeError(error, error_location, note, note_location) {}

        SyntaxError::SyntaxError(const std::string &error, const Location error_location,
                                 std::list<Diagnostic> &&note_list_)
            : CompileTimeError(error, error_location, std::move(note_list_)) {}

        void ErrorTracker::report_lexer_error(const std::string &error_message, Location error_location)
        {
                error_vector_.push_back(new LexerError(error_message, error_location));
        }

        void ErrorTracker::report_syntax_error(const std::string &error_message, Location error_location)
        {
                error_vector_.push_back(new SyntaxError(error_message, error_location));
        }

        void ErrorTracker::report_syntax_error(const std::string &error_message, Location error_location,
                                               const std::string &note, Location note_location)
        {
                error_vector_.push_back(new SyntaxError(error_message, error_location, note, note_location));
        }

        void ErrorTracker::report_syntax_error(const std::string &error_message, Location error_location,
                                               std::list<Diagnostic> &&note_list_)
        {
                error_vector_.push_back(new SyntaxError(error_message, error_location, std::move(note_list_)));
        }

        const std::vector<const CompileTimeError *> &ErrorTracker::ger_error_vector() const
        {
                return error_vector_;
        }
}