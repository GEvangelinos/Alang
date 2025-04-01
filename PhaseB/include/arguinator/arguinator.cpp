#include <cctype>
#include <format>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include "arguinator.hpp"

namespace
{
        std::string str_to_upper(const std::string &str)
        {
                std::stringstream ss;
                for (char ch : str)
                        ss << std::toupper(ch);
                return ss.str();
        }

        std::string str_to_lower(const std::string &str)
        {
                std::stringstream ss;
                for (char ch : str)
                        ss << std::tolower(ch);
                return ss.str();
        }

} /* namespace (Anonymous) */

namespace Arguinator
{
        Parser::Parser(int argc,
                       const char *const *argv,
                       const std::string &description,
                       bool case_sensitive)
            : argc_(argc),
              argv_(argv),
              description_(description),
              case_sensitive_(case_sensitive)
        {
                this->add_argument(ParserConsts::default_help_flag)
                    .set_help(ParserConsts::default_help_description)
                    .set_arity(0); /* Help does not require any inputs. */
        }

        Arg &Parser::add_argument(const std::string &identifier)
        {
                auto [it, inserted] = arg_map_.emplace(identifier, Arg());
                if (inserted)
                        return it->second; /* First field: key(identifier), second field: value (Arg) */

                throw std::logic_error(std::format(
                    "Duplicate argument flag `{}`.\n"
                    "Each flag must be registered only once.\n"
                    "Check for accidental reuse or typos in your argument declarations.",
                    it->first));
        }

        void Parser::parse_args()
        {
                const std::string executable_name = argv_[0];

                std::size_t arg_index = 1; /* We begin from index 1 to skip name of program. */
                while (arg_index < argc_)
                {
                        std::string current_arg_flag = argv_[arg_index]; /* First arg must be a flag (--argument-flag). */

                        if (!current_arg_flag.starts_with(ParserConsts::default_argument_flag_prefix))
                                throw std::runtime_error(std::format("Argument flags must be prefixed with {}, got {} (ex. {}argument-flag). ",
                                                                     ParserConsts::default_argument_flag_prefix,
                                                                     current_arg_flag,
                                                                     ParserConsts::default_argument_flag_prefix));
                        current_arg_flag.erase(0, 2); /* Remove the -- from the argument-flag. */
                        if (current_arg_flag == ParserConsts::default_help_flag)
                                this->display_help();

                        auto current_arg_record = arg_map_.find(current_arg_flag);
                        if (current_arg_record == arg_map_.end())
                                throw UnknownFlagError(std::format("Program `{}`, does not expect argument flag `{}`.",
                                                                   executable_name, current_arg_flag));
                        arg_index++;

                        const std::size_t expected_inputs = current_arg_record->second.get_arity();
                        std::size_t matched_inputs = 0;
                        while (arg_index < argc_ && matched_inputs < expected_inputs)
                        {
                                current_arg_record->second.add_input(argv_[arg_index]);
                                matched_inputs++;
                                arg_index++;
                        }
                        if (matched_inputs < expected_inputs)
                                throw ArityFlagError(std::format("Argument flag {} expected {} arguments, got {}. ",
                                                                 current_arg_flag, expected_inputs, matched_inputs));
                        current_arg_record->second.set_provided();
                }

                for (auto arg : arg_map_)
                {
                        std::vector<std::string> missing_arg_flags;
                        if (arg.second.is_required() && !arg.second.is_provided())
                                missing_arg_flags.push_back(arg.first);
                }
        }

        std::stringstream Parser::make_help()
        {
                const std::string executable_name = argv_[0];
                std::stringstream ss;

                ss << std::format("Usage: {} [options]", executable_name)
                   << '\n'
                   << description_
                   << '\n'
                   << "options:"
                   << '\n'
                   << std::left;

                /* Append the required inputs: */
                for (auto argument_flag : arg_map_)
                {
                        if (!argument_flag.second.is_required())
                                continue;

                        ss << '\t'
                           << std::setw(20)
                           << std::format("{}{}",
                                          ParserConsts::default_argument_flag_prefix,
                                          ParserConsts::default_help_flag);

                        const size_t required_inputs = argument_flag.second.get_arity();
                        for (auto input_index = 0; input_index < required_inputs; input_index++)
                        {
                                ss << str_to_upper(argument_flag.first);
                                if (required_inputs > 1)
                                        ss << "_" << std::to_string(input_index);
                        }
                        ss << '\n';
                }
                /* Append the none required inputs: */
                for (auto argument_flag : arg_map_)
                {
                        if (argument_flag.second.is_required())
                                continue;

                        ss << '\t'
                           << std::setw(20)
                           << std::format("[{}{}",
                                          ParserConsts::default_argument_flag_prefix,
                                          ParserConsts::default_help_flag);

                        const size_t required_inputs = argument_flag.second.get_arity();
                        for (auto input_index = 0; input_index < required_inputs; input_index++)
                        {
                                ss << str_to_upper(argument_flag.first);
                                if (required_inputs > 1)
                                        ss << "_" << std::to_string(input_index);
                        }
                        ss << ']' << '\n';
                }
                return ss;
        }

        Arg::Arg()
            : help_text_(ArgConsts::default_help_text),
              arity_(ArgConsts::default_arity),
              required_(ArgConsts::default_required) {}

        Arg &Arg::set_arity(const std::size_t no_inputs)
        {

                arity_ = no_inputs;
                return *this;
        }

        Arg &Arg::set_help(const std::string &help_text)
        {
                help_text_ = help_text;
                return *this;
        }

        Arg &Arg::set_required()
        {
                required_ = true;
                return *this;
        }

        void Arg::set_provided()
        {
                provided_ = true;
        }

        void Arg::add_input(const std::string &input)
        {
                inputs_.push_back(input);
        }

        std::size_t Arg::get_arity()
        {
                return arity_;
        }

        bool Arg::is_required()
        {
                return required_;
        }

        bool Arg::is_provided()
        {
                return provided_;
        }

        DuplicateFlagError::DuplicateFlagError(const std::string &error_message)
            : error_message_(error_message) {}

        const char *DuplicateFlagError::what() const noexcept
        {
                return error_message_.c_str();
        }

        ArityFlagError::ArityFlagError(const std::string &error_message)
            : error_message_(error_message) {}

        const char *ArityFlagError::what() const noexcept
        {
                return error_message_.c_str();
        }

        UnknownFlagError::UnknownFlagError(const std::string &error_message)
            : error_message_(error_message) {}

        const char *UnknownFlagError::what() const noexcept
        {
                return error_message_.c_str();
        }
} /* namespace Arguinator */
