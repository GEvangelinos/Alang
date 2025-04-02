#ifndef ARGUINATOR_HPP
#define ARGUINATOR_HPP

#include <string>
#include <map>
#include <stdexcept>
#include <vector>
#include "default_constants.hpp"

namespace Arguinator
{
        /* Forward declarations: */
        class Flag;
        class Parser;
        class FlagArityError;   /* Flag Exception */
        class FlagUnknownError; /* Flag Exception */
        class FlagMissingError; /* Flag Exception */

        class Parser
        {
        public:
                Parser(int argc,
                       const char *const *argv,
                       const std::string &description,
                       bool case_sensitive = ParserConsts::default_case_sensitive);
                Parser() = delete;

                Flag &set_flag(const std::string &identifier);
                void parse_flags();
                bool found(const std::string &flag_name) const;
                std::size_t count(const std::string &flag_name) const;

                const std::string &operator()(const std::string &flag_name, std::size_t input_field = 1);
                const std::vector<std::string> &operator[](const std::string &flag_name);

        private:
                const int argc_;
                const char *const *const argv_;
                const std::string description_;
                const bool case_sensitive_;
                std::map<std::string, Flag> flag_map_;

                void parse_flags_impl();
                std::string generate_help_text() const;
                [[noreturn]] void display_help_page();
        }; /* class Parser */

        class Flag
        {
        public:
                /* Fluent Builders */
                Flag &set_arity(std::size_t no_inputs) noexcept;
                Flag &set_help(const std::string &help_text) noexcept;
                Flag &set_required() noexcept;

                /* Modifiers: */
                void add_input(const std::string &input);
                void set_provided() noexcept;

                /* Accessors: */
                std::size_t get_arity() const noexcept;
                const std::string &get_help_text() const noexcept;
                bool is_required() const noexcept;
                bool is_provided() const noexcept;
                const std::string &get_name() const noexcept;
                const std::vector<std::string> &get_inputs() const noexcept;

                Flag() = delete;

        private:
                const std::string name_;
                std::vector<std::string> inputs_; /* Value(s) passed to flag. */
                std::size_t arity_;               /* Number of required inputs (e.g --rgb 255 255 0). */
                std::string help_text_;
                bool required_;
                bool provided_;

                Flag(const std::string &name); /* Defined, private, indirect use ONLY through set_flag(). */

                friend Flag &Parser::set_flag(const std::string &); /* Only function that can create Flags. */
        }; /* class Flag */

        class FlagError : public std::runtime_error
        {
        public:
                using std::runtime_error::runtime_error;
        };

        class FlagArityError : public FlagError
        {
        public:
                explicit FlagArityError(const std::string &flag_name,
                                        std::size_t expected_arity,
                                        std::size_t provided_arity);

        private:
                static std::string build_error_message(const std::string &flag_name,
                                                       std::size_t expected_arity,
                                                       std::size_t provided_arity);
        }; /* class FlagArityError */

        class FlagUnknownError : public FlagError
        {
        public:
                explicit FlagUnknownError(const std::string &flag_name);

        private:
                static std::string build_error_message(const std::string &flag_name);
        }; /* class FlagUnknownError */

        class FlagMissingError : public FlagError
        {
        public:
                FlagMissingError(const std::string &missing_flag);
                FlagMissingError(const std::vector<std::string> &missing_flags_vector);

        private:
                static std::string build_error_message(const std::vector<std::string> &missing_flags_vector);
        }; /* class FlagMissingError */

        class FlagFormatError : public FlagError
        {
        public:
                FlagFormatError(const std::string &flag_string);

        private:
                static std::string build_error_message(const std::string &flag_string);
        }; /* class FlagFormatError */
} /* namespace Arguinator */

#endif /* ARGUINATOR_HPP */
