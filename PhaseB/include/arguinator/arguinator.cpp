#include <cctype>
#include <format>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include "arguinator.hpp"

namespace /* (Anonymous) */
{
        std::string str_to_upper(std::string str)
        {
                std::transform(str.begin(), str.end(), str.begin(),
                               [](unsigned char c)
                               { return std::toupper(c); });
                return str;
        }

        std::string str_to_lower(std::string str)
        {
                std::transform(str.begin(), str.end(), str.begin(),
                               [](unsigned char c)
                               { return std::tolower(c); });
                return str;
        }

        void write_help_header(const std::string &executable_name,
                               const std::string &description,
                               std::stringstream &ss)
        {
                ss << std::format("Usage: {} [options]\n"
                                  "\n"
                                  "{}\n"
                                  "\n"
                                  "options:"
                                  "\n",
                                  executable_name, description);
        }

        std::string generate_example_values(const std::string &arg_name, std::size_t input_count)
        {
                const std::string base = str_to_upper(arg_name);
                if (input_count <= 1)
                        return base;

                /* Preallocate memory for efficient string building */
                constexpr auto separator_size = 1;     // (space)
                constexpr auto underscore_size = 1;    // _
                constexpr auto average_arity_size = 2; // assumes single ditgit arity.
                constexpr auto extra_per_input = separator_size + underscore_size + average_arity_size;

                std::string example;
                example.reserve(input_count * (base.size() + extra_per_input));

                for (std::size_t i = 0; i < input_count; i++)
                {
                        example += base;
                        example += "_";
                        example += std::to_string(i + 1); // 1-based indexing for example values
                        if (i < input_count - 1)
                                example += " ";
                }

                return example;
        }

        void write_help_argument_section(const std::map<std::string, Arguinator::Arg> &arg_map,
                                         const bool write_required,
                                         std::stringstream &ss)
        {
                constexpr auto prefix = Arguinator::ParserConsts::default_argument_flag_prefix;
                constexpr auto help_flag = Arguinator::ParserConsts::default_help_flag;

                for (const auto &arg : arg_map)
                {
                        const bool should_skip = write_required != arg.second.is_required();
                        if (should_skip)
                                continue;

                        const std::string example = generate_example_values(arg.first, arg.second.get_arity());
                        const std::string label = std::format("{}{} {}", prefix, arg.first, example);
                        const std::string wrapped_label = write_required
                                                              ? std::format("{}", label)
                                                              : std::format("[{}]", label);

                        constexpr auto arg_flag_slots = 20;
                        if (wrapped_label.length() <= arg_flag_slots) /* Based on length of label. */
                        {
                                /* One-line label and help text. */
                                ss << std::format("\t{:<{}} {}\n", wrapped_label, arg_flag_slots, arg.second.get_help_text());
                        }
                        else
                        {
                                /* Multi-line label and help text. */
                                ss << std::format("\t{}\n", wrapped_label);
                                ss << std::format("\t{:{}} {}\n", "", arg_flag_slots, arg.second.get_help_text());
                        }
                }
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
                this->set_argument(ParserConsts::default_help_flag)
                    .set_help(ParserConsts::default_help_description)
                    .set_arity(0); /* Help does not require any inputs. */
        }

        Arg &Parser::set_argument(const std::string &arg_name)
        {
                auto [it, inserted] = arg_map_.emplace(arg_name, Arg());
                if (inserted)
                        return it->second; /* First field: key(arg_name), second field: value (Arg) */

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
                                throw std::logic_error("DISPLAY_HELP"); // TODO : remove throw put functionf ro displaying help

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

        std::string Parser::generate_help_text() const
        {
                std::stringstream ss;

                write_help_header(argv_[0], description_, ss);

                /* Append the required inputs: */
                write_help_argument_section(arg_map_, true, ss);
                /* Append the none required inputs: */
                write_help_argument_section(arg_map_, false, ss);

                return ss.str();
        }

        Arg::Arg()
            : help_text_(ArgConsts::default_help_text),
              arity_(ArgConsts::default_arity),
              required_(ArgConsts::default_required) {}

        Arg &Arg::set_arity(const std::size_t no_inputs) noexcept
        {

                arity_ = no_inputs;
                return *this;
        }

        Arg &Arg::set_help(const std::string &help_text) noexcept
        {
                help_text_ = help_text;
                return *this;
        }

        Arg &Arg::set_required() noexcept
        {
                required_ = true;
                return *this;
        }

        void Arg::set_provided() noexcept
        {
                provided_ = true;
        }

        void Arg::add_input(const std::string &input)
        {
                inputs_.push_back(input);
        }

        std::size_t Arg::get_arity() const noexcept
        {
                return arity_;
        }

        const std::string &Arg::get_help_text() const noexcept
        {
                return help_text_;
        }

        bool Arg::is_required() const noexcept
        {
                return required_;
        }

        bool Arg::is_provided() const noexcept
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
