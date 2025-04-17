#ifndef ERROR_TRACKER_HPP
#define ERROR_TRACKER_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <list>
#include "core/alpha_location.hpp"
namespace Alpha
{

        struct CodeMessage
        {
                const std::string message_;
                const std::optional<Location> location_;
                CodeMessage(const std::string &message, std::optional<Location> location);
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
                virtual const std::string &to_string() const;

        protected:
                CodeMessage error_;
                std::list<CodeMessage> note_list;

                CompileTimeError(const std::string &error, Location error_location);

                CompileTimeError(const std::string &error, Location error_location,
                                 const std::string &note, Location note_location);

                CompileTimeError(const std::string &error, Location error_location,
                                std::list<CodeMessage> &&note_list);
        };

        class LexerError : public CompileTimeError
        {
        public:
                LexerError(const std::string &error, const Location error_location);

                LexerError() = delete;

                std::string get_error_type() const noexcept override { return "Lexer Error"; }
        };

        class SyntaxError : public CompileTimeError
        {
        public:
                SyntaxError(const std::string &error, const Location error_location);

                SyntaxError(const std::string &error, const Location error_location,
                            const std::string &note, const Location note_location);

                SyntaxError(const std::string &error, const Location error_location,
                            std::list<CodeMessage> &&note_list);

                SyntaxError() = delete;

                std::string get_error_type() const noexcept override { return "Syntax Error"; }
        };

        class ErrorTracker
        {
        public:
                ErrorTracker() = default;

                void register_lexer_error(const std::string &error, Location error_location);

                void register_syntax_error(const std::string &error, Location error_location);

                void register_syntax_error(const std::string &error, Location error_location,
                                           const std::string &note, Location note_location);

                void register_syntax_error(const std::string &error, Location error_location,
                                           std::list<CodeMessage> &&note_list);

                const std::vector<const CompileTimeError *> &ger_error_vector() const;

        private:
                std::vector<const CompileTimeError *> error_vector_;
        };
}

#endif /* ERROR_TRACKER_HPP */
