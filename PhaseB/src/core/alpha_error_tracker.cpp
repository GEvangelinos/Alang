#include <format>
#include "core/alpha_error_tracker.hpp"

namespace Alpha
{
        CodeMessage::CodeMessage(const std::string &message, std::optional<Location> location)
            : message_(message), location_(location) {}

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

        const std::string &CompileTimeError::to_string() const
        {
                // TODO: Implement
                ;
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