#ifndef ARGUINATOR_HPP
#define ARGUINATOR_HPP

#include <string>
#include <map>
#include <exception>
#include <vector>
#include "default_constants.hpp"

namespace Arguinator
{
        /* Forward declarations: */
        class Arg;
        class Parser;
        class DuplicateFlagError; /* Runtime Exception */
        class ArityFlagError;     /* Runtime Exception */

        class Parser
        {
        public:
                Parser(int argc,
                       const char *const *argv,
                       const std::string &description,
                       bool case_sensitive = ParserConsts::default_case_sensitive);
                Parser() = delete;

                Arg &add_argument(const std::string &identifier);
                void parse_args();
                std::stringstream make_help();

        private:
                const int argc_;
                const char *const *const argv_;
                const std::string description_;
                const bool case_sensitive_;
                std::map<std::string, Arg> arg_map_;
        }; /* class Parser */

        class Arg
        {
        public:
                /* Fluent Builders */
                Arg &set_arity(size_t no_inputs) noexcept;
                Arg &set_help(const std::string &help_text) noexcept;
                Arg &set_required() noexcept;

                /* Modifiers: */
                void add_input(const std::string &input);
                void set_provided();

                /* Accessors: */
                size_t get_arity();
                bool is_required();
                bool is_provided();

        private:
                std::vector<std::string> inputs_; /* Value passed to argument. */
                std::string help_text_;
                size_t arity_; /* Number of required inputs (e.g --rgb 255 255 0). */
                bool required_;
                bool provided_;

                Arg(); /* Defined, private, indirect use ONLY through add_argument. */

                friend Arg &Parser::add_argument(const std::string &); /* Only function that can create Args. */
        }; /* class Arg */

        class DuplicateFlagError : std::exception
        {
        public:
                explicit DuplicateFlagError(const std::string &error_message);

                const char *what() const noexcept override;

        private:
                std::string error_message_;
        }; /* class DuplicateFlagError */

        class ArityFlagError : std::exception
        {
        public:
                explicit ArityFlagError(const std::string &error_message);

                const char *what() const noexcept override;

        private:
                std::string error_message_;
        }; /* class ArityFlagError */

        class UnknownFlagError : std::exception
        {
        public:
                explicit UnknownFlagError(const std::string &error_message);

                const char *what() const noexcept override;

        private:
                std::string error_message_;
        }; /* class UnknownFlagError */
} /* namespace Arguinator */

#endif /* ARGUINATOR_HPP */