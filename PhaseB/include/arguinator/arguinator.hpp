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
                       const std::string &description);
                Parser() = delete;

                Flag &set_flag(const std::string &identifier);
                void parse_flags();
                std::size_t count(const std::string &flag_name) const;

                const Flag &operator[](const std::string &flag_name) const;

        private:
                std::size_t argc_;
                const char *const *const argv_;
                const std::string description_;
                std::map<std::string, Flag> flag_map_;

                bool found(const std::string &flag_name) const;

                void parse_flags_impl();
                std::string generate_help_text() const;
                [[noreturn]] void display_help_page();
        }; /* class Parser */

        class Flag
        {
        public:
                /* Rule of 5 */
                Flag(const Flag &other) = default;
                Flag(Flag &&other) noexcept = delete;
                Flag &operator=(Flag &&other) noexcept = delete;
                ~Flag() noexcept = default;

                Flag() = delete;

                /* Fluent Builders */
                Flag &set_arity(std::size_t no_inputs) noexcept;
                Flag &set_help(const std::string &help_text) noexcept;
                Flag &set_required() noexcept;

                /* Modifiers */
                void add_input(const std::string &input);

                /* Accessors */
                std::size_t get_arity() const noexcept;
                const std::string &get_help_text() const noexcept;
                bool is_required() const noexcept;
                bool is_provided() const noexcept;
                const std::string &get_name() const noexcept;
                const std::vector<std::string> &get_inputs() const noexcept;

                /* By default it returns first input, helpful in argument with arity 1*/
                const std::string &get_input(std::size_t input_field = 1) const;

        private:
                /* Private constructor */
                Flag(const std::string &name); /* Only usable via set_flag() */

                /* Internal modifiers */
                void set_provided() noexcept;

                const std::string name_;
                std::size_t arity_; /* Number of required inputs (e.g. --rgb 255 255 0) */
                std::string help_text_;
                bool required_;
                bool provided_;
                std::vector<std::string> inputs_; /* Values passed to flag */

                friend class Parser;
        };

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
