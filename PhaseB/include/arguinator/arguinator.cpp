#include <cctype>
#include <cstring>
#include <format>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "arguinator.hpp"

namespace /* (Anonymous) */
{
        using namespace Arguinator;

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

        std::string generate_example_values(const std::string &flag_name, std::size_t input_count)
        {
                const std::string base = str_to_upper(flag_name);
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

        void write_help_flag_section(const std::map<std::string, Flag> &flag_map,
                                     const bool write_required,
                                     std::stringstream &ss)
        {
                constexpr auto prefix = ParserConsts::default_flag_prefix;
                constexpr auto help_flag = ParserConsts::default_help_flag;

                for (const auto &flag_entry : flag_map)
                {
                        const std::string &flag_name = flag_entry.first;
                        const Flag &flag = flag_entry.second;

                        const bool should_skip = write_required != flag.is_required();
                        if (should_skip)
                                continue;

                        const std::string example = generate_example_values(flag_name, flag.get_arity());
                        const std::string label = std::format("{}{} {}", prefix, flag_name, example);
                        const std::string wrapped_label = write_required
                                                              ? std::format("{}", label)
                                                              : std::format("[{}]", label);

                        if (wrapped_label.length() <= ParserConsts::default_flag_field_size)
                                ss << std::format("\t{:<{}} {}\n", wrapped_label, ParserConsts::default_flag_field_size, flag.get_help_text());
                        else
                        {
                                /* Multi-line label and help text. */
                                ss << std::format("\t{}\n", wrapped_label);
                                ss << std::format("\t{:{}} {}\n", "", ParserConsts::default_flag_field_size, flag.get_help_text());
                        }
                }
        }

        void assert_flag_format(const std::string &flag_string)
        {
                if (!flag_string.starts_with(ParserConsts::default_flag_prefix))
                        throw FlagFormatError(flag_string);
        }

        void strip_flag_prefix(std::string &flag)
        {
                constexpr auto prefix_len = std::strlen(ParserConsts::default_flag_prefix);
                flag.erase(0, prefix_len); /* Remove the -- from the flag. */
        }

        void ensure_known_flag(const std::string &flag_name, const std::map<std::string, Flag> &flag_map)
        {
                if (!flag_map.contains(flag_name))
                        throw FlagUnknownError(flag_name);
        }

        void process_flag_inputs(const int argc,
                                 const char *const *const argv,
                                 Flag &flag,
                                 std::size_t &flag_index)
        {
                const std::size_t expected_inputs = flag.get_arity();
                std::size_t matched_inputs = 0;
                while (flag_index < argc && matched_inputs < expected_inputs)
                {
                        flag.add_input(argv[flag_index]);
                        matched_inputs++;
                        flag_index++;
                }
                if (matched_inputs < expected_inputs)
                        throw FlagArityError(flag.get_name(), expected_inputs, matched_inputs);
                flag.set_provided();
        }

        void ensure_required_flags_present(std::map<std::string, Flag> flag_map)
        {
                std::vector<std::string> missing_flags_vector; /* Flags that are required but missing. */
                for (auto flag_entry : flag_map)
                {
                        if (flag_entry.second.is_required() && !flag_entry.second.is_provided())
                                missing_flags_vector.push_back(flag_entry.first);
                }
                if (!missing_flags_vector.empty())
                        throw FlagMissingError(missing_flags_vector);
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
                /* Parser must always contain the --help flag. */
                this->set_flag(ParserConsts::default_help_flag)
                    .set_help(ParserConsts::default_help_description)
                    .set_arity(0); /* Help does not require any inputs. */
        }

        Flag &Parser::set_flag(const std::string &flag_name)
        {
                auto [it, inserted] = flag_map_.emplace(flag_name, Flag(flag_name));
                if (inserted)
                        return it->second; /* First field: key(flag_name), second field: value (Flag) */

                throw std::logic_error(std::format(
                    "Duplicate flag `{}`.\n"
                    "Each flag must be registered only once.\n"
                    "Check for accidental reuse or typos in your flag declarations.",
                    it->first));
        }

        void Parser::parse_flags_impl()
        {
                std::size_t flag_index = 1; /* We begin from index 1 to skip name of program. */
                while (flag_index < argc_)
                {
                        std::string current_flag_name = argv_[flag_index]; /* First argument must be a flag (--example-flag). */

                        assert_flag_format(current_flag_name);
                        strip_flag_prefix(current_flag_name);
                        if (current_flag_name == ParserConsts::default_help_flag)
                                display_help_page(); /* NORETURN */
                        ensure_known_flag(current_flag_name, flag_map_);
                        auto &current_flag = flag_map_.find(current_flag_name)->second;
                        process_flag_inputs(argc_, argv_, current_flag, ++flag_index);
                }
                ensure_required_flags_present(flag_map_);
        }

        void Parser::parse_flags()
        {
                try
                {
                        this->parse_flags_impl();
                }
                catch (FlagError &e)
                {
                        std::cerr << e.what() << std::endl;
                }
        }

        bool Parser::found(const std::string &flag_name) const
        {
                const std::string flag_string = ParserConsts::default_flag_prefix + flag_name;
                if (!flag_map_.contains(flag_name))
                        throw std::out_of_range(std::format("Flag {} not registered!", flag_string));

                return flag_map_.at(flag_name).is_provided();
        }

        std::size_t Parser::count(const std::string &flag_name) const
        {
                if (this->found(flag_name))
                        return flag_map_.at(flag_name).get_inputs().size();

                const std::string flag_string = ParserConsts::default_flag_prefix + flag_name;
                throw std::out_of_range(std::format("Flag {} not registered!", flag_string));
        }

        const std::string &Parser::operator()(const std::string &flag_name, std::size_t input_field)
        {
                if (input_field == 0)
                        std::range_error("Indexing field starts from 1, not 0");
                const std::string flag_string = ParserConsts::default_flag_prefix + flag_name;
                if (!flag_map_.contains(flag_name))
                        throw std::out_of_range(std::format("Flag {} not registered!", flag_string));

                const Flag &flag = flag_map_.at(flag_name);
                if (input_field > flag.get_arity())
                        throw std::out_of_range(std::format("Flag {} has arity {} but field #{} requested.",
                                                            flag_string, flag.get_arity(), input_field));
                return flag.get_inputs().at(input_field - 1); /* -1 to convert input_field to vector index. */
        }

        const std::vector<std::string> &Parser::operator[](const std::string &flag_name)
        {
                const std::string flag_string = ParserConsts::default_flag_prefix + flag_name;
                if (!flag_map_.contains(flag_name))
                        throw std::out_of_range(std::format("Flag {} not registered!", flag_string));
                return flag_map_.at(flag_name).get_inputs();
        }

        std::string Parser::generate_help_text() const
        {
                std::stringstream ss;

                write_help_header(argv_[0], description_, ss);

                /* Append the required inputs: */
                write_help_flag_section(flag_map_, true, ss);
                /* Append the none required inputs: */
                write_help_flag_section(flag_map_, false, ss);

                return ss.str();
        }

        [[noreturn]] void Parser::display_help_page()
        {
                std::cout << this->generate_help_text();
                std::exit(0);
        }

        Flag::Flag(const std::string &name)
            : name_(name),
              help_text_(FlagConsts::default_help_text),
              arity_(FlagConsts::default_arity),
              required_(FlagConsts::default_required) {}

        Flag &Flag::set_arity(const std::size_t no_inputs) noexcept
        {

                arity_ = no_inputs;
                return *this;
        }

        Flag &Flag::set_help(const std::string &help_text) noexcept
        {
                help_text_ = help_text;
                return *this;
        }

        Flag &Flag::set_required() noexcept
        {
                required_ = true;
                return *this;
        }

        void Flag::set_provided() noexcept
        {
                provided_ = true;
        }

        void Flag::add_input(const std::string &input)
        {
                inputs_.push_back(input);
        }

        std::size_t Flag::get_arity() const noexcept
        {
                return arity_;
        }

        const std::string &Flag::get_help_text() const noexcept
        {
                return help_text_;
        }

        bool Flag::is_required() const noexcept
        {
                return required_;
        }

        bool Flag::is_provided() const noexcept
        {
                return provided_;
        }

        const std::string &Flag::get_name() const noexcept
        {
                return name_;
        }

        const std::vector<std::string> &Flag::get_inputs() const noexcept
        {
                return inputs_;
        }

        FlagArityError::FlagArityError(const std::string &flag_name,
                                       std::size_t expected_arity,
                                       std::size_t provided_arity)
            : FlagError(build_error_message(flag_name, expected_arity, provided_arity)) {}

        std::string FlagArityError::build_error_message(const std::string &flag_name,
                                                        std::size_t expected_arity,
                                                        std::size_t provided_arity)
        {
                return std::format("{}{} expected {} inputs, got {}. ",
                                   ParserConsts::default_flag_prefix,
                                   flag_name,
                                   expected_arity,
                                   provided_arity);
        }

        FlagUnknownError::FlagUnknownError(const std::string &flag_name)
            : FlagError(build_error_message(flag_name)) {}

        std::string FlagUnknownError::build_error_message(const std::string &flag_name)
        {
                return std::format("{}{} flag is unknown", ParserConsts::default_flag_prefix, flag_name);
        }

        FlagMissingError::FlagMissingError(const std::string &error_message)
            : FlagError(error_message) {}

        FlagMissingError::FlagMissingError(const std::vector<std::string> &missing_flags_vector)
            : FlagError(build_error_message(missing_flags_vector)) {}

        std::string FlagMissingError::build_error_message(const std::vector<std::string> &missing_flags_vector)
        {
                std::stringstream ss;
                ss << "Arguinator error, the following flags are required:\n";
                for (auto flag_name : missing_flags_vector)
                        ss << '\t' << ParserConsts::default_flag_prefix << flag_name << '\n';
                ss << "Use flag " << ParserConsts::default_flag_prefix << ParserConsts::default_help_flag << " to display options menu.";
                return ss.str();
        }

        FlagFormatError::FlagFormatError(const std::string &flag_string)
            : FlagError(build_error_message(flag_string)) {}

        std::string FlagFormatError::build_error_message(const std::string &flag_string)
        {
                return std::format("Flags must be prefixed with {}. Got: {}",
                                   ParserConsts::default_flag_prefix, flag_string);
        }
} /* namespace Arguinator */
