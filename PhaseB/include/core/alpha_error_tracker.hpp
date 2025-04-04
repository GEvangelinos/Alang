#ifndef ERROR_TRACKER_HPP
#define ERROR_TRACKER_HPP

#include <string>
#include <vector>
#include <cstdint>
#include "core/alpha_location.hpp"
namespace Alpha
{
        /* TODO: implement clang-gcc like error reporting. */
        class CompileTimeError
        {
        protected:
                const SourceRange error_range_;
                const std::string error_message_;

        public:
                CompileTimeError(const SourceRange &error_range, const std::string &error_message);
                CompileTimeError() = delete;
                virtual std::string get_error_type() const = 0;
                virtual std::string to_string() const;
        };

        class LexerError : public CompileTimeError
        {
        public:
                LexerError(const Location &error_location, const std::string &error_message);
                LexerError(const SourceRange &error_range, const std::string &error_message);
                LexerError() = delete;

                std::string get_error_type() const override { return "Lexer Error"; }
        };

        class SyntaxError : public CompileTimeError
        {
        public:
                SyntaxError(const Location &error_location, const std::string &error_message);
                SyntaxError(const SourceRange &errro_range, const std::string &error_message);
                SyntaxError() = delete;

                std::string get_error_type() const override { return "Syntax Error"; }
        };

        class ErrorTracker
        {
        public:
                ErrorTracker() = default;

                void register_compile_time_error(const CompileTimeError *error);
                const std::vector<const CompileTimeError *> &ger_error_vector() const;

        private:
                std::vector<const CompileTimeError *> error_vector_;
        };
}

#endif /* ERROR_TRACKER_HPP */
