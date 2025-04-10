#include <format>
#include "core/alpha_error_tracker.hpp"

namespace Alpha
{
        CompileTimeError::CodeMessage::CodeMessage(const std::string &message, std::optional<CodeLocation> location)
            : message_(message), location_(location) {}

        CompileTimeError::CompileTimeError(const std::string &error_message, CodeLocation error_location,
                                           const std::string &note_message, CodeLocation note_location)
            : error_(error_message, error_location),
              notes_{{note_message, note_location}} {}

        CompileTimeError::CompileTimeError(const std::string &error_message, CodeLocation error_location)
            : error_(error_message, error_location) {}

        const std::string &CompileTimeError::to_string() const
        {
                // TODO: Implement
                ;
        }

        LexerError::LexerError(const std::string &error_message, CodeLocation error_location)
            : CompileTimeError(error_message, error_location) {}

        SyntaxError::SyntaxError(const std::string &error_message, CodeLocation error_location)
            : CompileTimeError(error_message, error_location) {}

        SyntaxError::SyntaxError(const std::string &error, const CodeLocation error_location,
                                 const std::string &note, const CodeLocation note_location)
            : CompileTimeError(error, error_location, note, note_location) {}

        void ErrorTracker::register_lexer_error(const std::string &error_message, CodeLocation error_location)
        {
                error_vector_.push_back(new LexerError(error_message, error_location));
        }

        void ErrorTracker::register_syntax_error(const std::string &error_message, CodeLocation error_location)
        {
                error_vector_.push_back(new SyntaxError(error_message, error_location));
        }

        void ErrorTracker::register_syntax_error(const std::string &error_message, CodeLocation error_location,
                                                 const std::string &note, CodeLocation note_location)
        {
                error_vector_.push_back(new SyntaxError(error_message, error_location, note, note_location));
        }

        const std::vector<const CompileTimeError *> &ErrorTracker::ger_error_vector() const
        {
                return error_vector_;
        }
}