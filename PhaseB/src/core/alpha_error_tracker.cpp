#include "utils/format_adapter.hpp"
#include "core/alpha_error_tracker.hpp"

#include <sstream>
#include "utils/cli_color.h"
#include "utils/format_adapter.hpp"
#include "core/alpha_location.hpp"
#include <iostream>
#include <iomanip>

namespace Alpha
{
        CodeMessage::CodeMessage(const std::string &message, std::optional<Location> location)
            : message(message), location(location) {}

        CompileTimeError::CompileTimeError(const std::string &error_message, Location error_location)
            : error_(error_message, error_location) {}

        CompileTimeError::CompileTimeError(const std::string &error_message, Location error_location,
                                           const std::string &note_message, Location note_location)
            : error_(error_message, error_location),
              note_list{{note_message, note_location}} {}

        CompileTimeError::CompileTimeError(const std::string &error_message, Location error_location,
                                           std::list<CodeMessage> &&note_list)
            : error_(error_message, error_location),
              note_list(std::move(note_list)) {}

        std::string CompileTimeError::assemble_code_message(
            const std::string &source_filename,
            const LocationTracker &lt,
            const char *input_buffer) const
        {
        }

        std::string CompileTimeError::to_string(
            const std::string &source_filename,
            const LocationTracker &lt,
            const char *buffer_input) const
        {
                std::stringstream ss;
                ss << source_filename;

                if (error_.location.has_value())
                {
                        auto loc = error_.location.value();
                        int line = lt.find_first_line(loc);
                        int column = lt.find_first_column(loc);
                        ss << ":" << line << ":" << column;
                }

                ss << ": "
                   << COLOR_ASCII_FG_RED
                   << this->get_error_type() << " error"
                   << SGR_RESET
                   << ": "
                   << error_.message;

                if (error_.location.has_value())
                {
                        auto loc = error_.location.value();
                        int line = lt.find_first_line(loc);
                        int column = lt.find_first_column(loc);
                        std::size_t line_start_index = lt.find_index_at_line(line);
                        const char *line_start = &buffer[line_start_index];
                        const char *line_end_ptr = std::strchr(line_start, '\n');
                        std::string_view line_view(line_start, line_end_ptr ? line_end_ptr - line_start : std::strlen(line_start));

                        int line_digits = std::to_string(line).size();
                        int pad_width = line_digits > 8 ? line_digits : 8;

                        ss << '\n'
                           << std::setw(pad_width) << line << " | " << line_view << "\n"
                           << std::setw(pad_width) << ' ' << " | " << std::string(column - 1, ' ') << "^";
                }

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
                                 std::list<CodeMessage> &&note_list)
            : CompileTimeError(error, error_location, std::move(note_list)) {}

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
                                               std::list<CodeMessage> &&note_list)
        {
                error_vector_.push_back(new SyntaxError(error_message, error_location, std::move(note_list)));
        }

        const std::vector<const CompileTimeError *> &ErrorTracker::ger_error_vector() const
        {
                return error_vector_;
        }
}