#include <format>
#include "core/alpha_error_tracker.hpp"

namespace Alpha
{
    CompileTimeError::CompileTimeError(const SourceRange &error_range, const std::string &error_message)
        : error_range_(error_range), error_message_(error_message) {}

    std::string CompileTimeError::to_string() const
    {
        return std::format("{} {}", this->get_error_type(), error_message_); // TODO: this is outdated, write g++/clang++ style errors.
    }

    LexerError::LexerError(const Location &error_location, const std::string &error_message)
        : LexerError(SourceRange(error_location), error_message) {} /* Implicit conversion (I have special constructor). */

    LexerError::LexerError(const SourceRange &error_range, const std::string &error_message)
        : CompileTimeError(error_range, error_message) {}

    SyntaxError::SyntaxError(const Location &error_location, const std::string &error_message)
        : SyntaxError(SourceRange(error_location), error_message) {} /* Implicit conversion (I have special constructor). */

    SyntaxError::SyntaxError(const SourceRange &error_range, const std::string &error_message)
        : CompileTimeError(error_range, error_message) {}

    void ErrorTracker::register_compile_time_error(const CompileTimeError *const error)
    {
        error_vector_.push_back(error);
    }

    const std::vector<const CompileTimeError *> &ErrorTracker::ger_error_vector() const
    {
        return error_vector_;
    }
}