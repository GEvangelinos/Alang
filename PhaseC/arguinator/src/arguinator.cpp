#include "arguinator/arguinator.hpp"
#include <algorithm>                // for transform
#include <cctype>                   // for tolower, toupper
#include <cstdlib>                  // for exit
#include <iostream>                 // for basic_ostream, operator<<
#include <sstream>                  // for basic_stringstream
#include <stdexcept>                // for out_of_range, logic_error
#include <utility>                  // for pair
#include "arguinator/defaults.hpp"  // for default_flag_prefix, def...
#include "utils/format_adapter.hpp" // for format, FMT
#include "utils/smart_assert.h"     // for SMART_ASSERT

#define ATTACH_CONTEXT(message)                                                                    \
        FMT::format("[{}:{}:{}]\n{}", __FILE__, __LINE__, __func__, (message))

#define TO_STRING(x) #x

namespace arguinator
{
class FlagError : public CLIError
{
public:
    explicit FlagError(const std::string &e) : CLIError(e) {}
};

class FlagArityError final : public FlagError
{
public:
    FlagArityError(const std::string &flag_name, std::size_t expected_arity,
                   std::size_t provided_arity);

private:
    [[nodiscard]] static std::string build_error_message(const std::string &flag_name,
                                                         std::size_t expected_arity,
                                                         std::size_t provided_arity);
}; /* class FlagArityError */

class FlagUnknownError final : public FlagError
{
public:
    explicit FlagUnknownError(const std::string &flag_name);

private:
    [[nodiscard]] static std::string build_error_message(const std::string &flag_name);
}; /* class FlagUnknownError */

class FlagMissingError final : public FlagError
{
public:
    explicit FlagMissingError(const std::string &missing_flag);

    explicit FlagMissingError(const std::vector<std::string> &missing_flags_vector);

private:
    [[nodiscard]] static std::string build_error_message(
        const std::vector<std::string> &missing_flags_vector);
}; /* class FlagMissingError */

class FlagFormatError final : public FlagError
{
public:
    explicit FlagFormatError(const std::string &flag_string);

private:
    [[nodiscard]] static std::string build_error_message(const std::string &flag_string);
}; /* class FlagFormatError */
}  // namespace Arguinator

namespace
{
using namespace arguinator;

[[nodiscard]] std::string str_to_upper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return str;
}

[[maybe_unused]] [[nodiscard]] std::string str_to_lower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return str;
}

void write_help_header(const std::string &executable_name, const std::string &description,
                       std::stringstream &ss)
{
    ss << FMT::format("Usage: {} [options]\n"
                      "\n"
                      "{}\n"
                      "\n"
                      "options:"
                      "\n",
                      executable_name, description);
}

[[nodiscard]] std::string generate_example_values(const std::string &flag_name,
                                                  const std::size_t arity)
{
    const std::string base = str_to_upper(flag_name);
    if (arity == 0)
        return ""; // No inputs expected → no example
    if (arity == 1)
        return base; // Single input → just show flag name

    /* Preallocate memory for efficient string building */
    constexpr auto separator_size = 1;     // (space)
    constexpr auto underscore_size = 1;    // _
    constexpr auto average_arity_size = 2; // assumes single digit arity.
    constexpr auto extra_per_input = separator_size + underscore_size + average_arity_size;

    std::string example;
    example.reserve(arity * (base.size() + extra_per_input));

    for (std::size_t i = 0; i < arity; i++)
    {
        example += base;
        example += "_";
        example += std::to_string(i + 1); // 1-based indexing for example values
        if (i < arity - 1)
            example += " ";
    }
    return example;
}

void write_help_flag_section(const std::map<std::string, Flag> &flag_map, const bool write_required,
                             std::stringstream &ss)
{
    for (const auto &flag_entry: flag_map)
    {
        const std::string &flag_name = flag_entry.first;
        const Flag &flag = flag_entry.second;

        if (write_required != flag.is_required())
            continue;

        const std::string example = generate_example_values(flag_name, flag.get_arity());
        const std::string label =
                FMT::format("{}{} {}", ParserConsts::default_flag_prefix, flag_name,
                            example);
        const std::string wrapped_label =
                write_required ? FMT::format("{}", label) : FMT::format("[{}]", label);

        if (wrapped_label.length() <= ParserConsts::default_flag_field_size)
            ss << FMT::format("\t{:<{}} {}\n", wrapped_label,
                              ParserConsts::default_flag_field_size,
                              flag.get_help_text());
        else
        {
            /* Multi-line label and help text. */
            ss << FMT::format("\t{}\n", wrapped_label);
            ss << FMT::format("\t{:{}} {}\n", "", ParserConsts::default_flag_field_size,
                              flag.get_help_text());
        }
        ss << '\n';
    }
}

void assert_flag_format(const std::string &flag_string)
{
    if (!flag_string.starts_with(ParserConsts::default_flag_prefix))
        throw FlagFormatError(flag_string);
}

void strip_flag_prefix(std::string &flag)
{
    constexpr auto prefix_len =
            sizeof(ParserConsts::default_flag_prefix) - 1;
    /* -1, cause sizeof() count '\0' too. */
    flag.erase(0, prefix_len); /* Remove the -- from the flag. */
}

void ensure_known_flag(const std::string &flag_name, const std::map<std::string, Flag> &flag_map)
{
    if (!flag_map.contains(flag_name))
        throw FlagUnknownError(flag_name);
}

void process_flag_inputs(const std::size_t argc, const char *const *const argv, Flag &flag,
                         std::size_t &flag_index)
{
    const std::size_t expected_inputs = flag.get_arity();
    std::size_t matched_inputs = 0;
    while (flag_index < argc && matched_inputs < expected_inputs)
    {
        const std::string input_value = argv[flag_index];
        if (input_value.starts_with(ParserConsts::default_flag_prefix))
            break;
        flag.add_input(input_value);
        matched_inputs++;
        flag_index++;
    }
    if (matched_inputs < expected_inputs)
        throw FlagArityError(flag.get_name(), expected_inputs, matched_inputs);
}

void ensure_required_flags_present(std::map<std::string, Flag> flag_map)
{
    std::vector<std::string> missing_flags_vector; /* Flags that are required but missing. */
    for (auto flag_entry: flag_map)
        if (flag_entry.second.is_required() && !flag_entry.second.is_provided())
            missing_flags_vector.push_back(flag_entry.first);
    if (!missing_flags_vector.empty())
        throw FlagMissingError(missing_flags_vector);
}
} // namespace

namespace arguinator
{
Parser::Parser(int argc, const char *const *argv, const std::string &description)
    : argc_(argc), argv_(argv), description_(description)
{
    /* We want to assert, because internally we convert to std::size_t, plus argc <= 0 makes no
     * sense. */
    SMART_ASSERT(argc > 0, !!argv);

    /* Parser must always contain the --help flag. */
    this->set_flag(ParserConsts::default_help_flag)
         .set_help(ParserConsts::default_help_text)
         .set_arity(0); /* Help does not require any inputs. */
}

Flag &Parser::set_flag(const std::string &flag_name)
{
    auto [it, inserted] = flag_map_.emplace(flag_name, Flag(flag_name));
    if (inserted)
        return it->second; /* First field: key(flag_name), second field: value (Flag) */

    // If not inserted, the flag already existed
    throw std::logic_error(ATTACH_CONTEXT(
        FMT::format("BUG: Duplicate flag `{}`.\n"
            "Each flag must be registered only once.\n"
            "Check for accidental reuse or typos in your flag declarations.",
            it->first)));
}

void Parser::parse_flags_impl()
{
    std::size_t flag_index = 1; /* We begin from index 1 to skip name of program. */
    while (flag_index < argc_)
    {
        std::string current_flag_name =
                argv_[flag_index]; /* First argument must be a flag (--example-flag). */

        assert_flag_format(current_flag_name);
        strip_flag_prefix(current_flag_name);
        if (current_flag_name == ParserConsts::default_help_flag)
            display_help_page(); /* NORETURN */
        ensure_known_flag(current_flag_name, flag_map_);
        auto &current_flag = flag_map_.find(current_flag_name)->second;
        process_flag_inputs(argc_, argv_, current_flag, ++flag_index);
        current_flag.set_provided();
    }
    ensure_required_flags_present(flag_map_);
}

void Parser::parse_flags() { parse_flags_impl(); }

bool Parser::found(const std::string &flag_name) const { return flag_map_.contains(flag_name); }

std::size_t Parser::count(const std::string &flag_name) const
{
    const std::string flag_string = ParserConsts::default_flag_prefix + flag_name;

    if (found(flag_name))
    {
        if (flag_map_.at(flag_name).is_provided())
            return flag_map_.at(flag_name).get_inputs().size();
        throw std::logic_error(ATTACH_CONTEXT(FMT::format(
            "BUG: {}() called for unprovided flag `{}`. Only provided flags should be used.",
            __func__, flag_string)));
    }

    throw std::logic_error(ATTACH_CONTEXT(FMT::format(
        "BUG: {}() called for unregistered flag `{}`. Only registered flags should be used."
       ,
        __func__, flag_string)));
}

const Flag &Parser::operator[](const std::string &flag_name) const
{
    const std::string flag_string = ParserConsts::default_flag_prefix + flag_name;

    if (found(flag_name))
        return flag_map_.at(flag_name);

    throw std::logic_error(ATTACH_CONTEXT(FMT::format(
        "BUG: {} called for unregistered flag `{}`. Only registered flags may be accessed.",
        __func__, flag_string)));
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

void Parser::display_help_page()
{
    std::cout << generate_help_text();
    throw CLIHelp();
}

Flag::Flag(const std::string &name)
    : name_(name),
      arity_(FlagConsts::default_arity),
      help_text_(FlagConsts::default_help_text),
      required_(FlagConsts::default_required),
      provided_(false) {}

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

void Flag::set_provided() noexcept { provided_ = true; }

void Flag::add_input(const std::string &input) { inputs_.push_back(input); }

std::size_t Flag::get_arity() const noexcept { return arity_; }

const std::string &Flag::get_help_text() const noexcept { return help_text_; }

bool Flag::is_required() const noexcept { return required_; }

bool Flag::is_provided() const noexcept { return provided_; }

const std::string &Flag::get_name() const noexcept { return name_; }

const std::vector<std::string> &Flag::get_inputs() const noexcept { return inputs_; }

const std::string &Flag::get_input(std::size_t input_field) const
{
    if (input_field == 0)
        throw std::logic_error(ATTACH_CONTEXT(FMT::format(
            "BUG: {}() called with input_field = 0, which is invalid. Indexing starts from 1.",
            __func__
        )));

    const std::string flag_string = ParserConsts::default_flag_prefix + name_;
    if (input_field > arity_)
        throw std::logic_error(ATTACH_CONTEXT(FMT::format(
            "BUG: Flag's `{}` registered arity is `{}`, but field `#{}` was requested.",
            flag_string, arity_, input_field)));

    return inputs_.at(input_field - 1); // -1 to convert 1-based input_field to 0-based index
}

FlagArityError::FlagArityError(const std::string &flag_name, std::size_t expected_arity,
                               std::size_t provided_arity)
    : FlagError(build_error_message(flag_name, expected_arity, provided_arity)) {}

std::string FlagArityError::build_error_message(const std::string &flag_name,
                                                std::size_t expected_arity,
                                                std::size_t provided_arity)
{
    return FMT::format("{}: {}{} expected {} inputs. Got {}. ", TO_STRING(FlagArityError),
                       ParserConsts::default_flag_prefix, flag_name, expected_arity,
                       provided_arity);
}

FlagUnknownError::FlagUnknownError(const std::string &flag_name)
    : FlagError(build_error_message(flag_name)) {}

std::string FlagUnknownError::build_error_message(const std::string &flag_name)
{
    return FMT::format("{}{} flag is unknown", ParserConsts::default_flag_prefix, flag_name);
}

FlagMissingError::FlagMissingError(const std::string &error_message)
    : FlagError(error_message) {}

FlagMissingError::FlagMissingError(const std::vector<std::string> &missing_flags_vector)
    : FlagError(build_error_message(missing_flags_vector)) {}

std::string FlagMissingError::build_error_message(
    const std::vector<std::string> &missing_flags_vector)
{
    std::stringstream ss;
    ss << "Arguinator error, the following flags are required:\n";
    for (auto &flag_name: missing_flags_vector)
        ss << '\t' << ParserConsts::default_flag_prefix << flag_name << '\n';
    ss << '\n';
    ss << FMT::format("!! Use flag{} to display options menu.",
                      ParserConsts::default_flag_prefix, ParserConsts::default_help_flag);
    return ss.str();
}

FlagFormatError::FlagFormatError(const std::string &flag_string)
    : FlagError(build_error_message(flag_string)) {}

std::string FlagFormatError::build_error_message(const std::string &flag_string)
{
    return FMT::format("Flags must be prefixed with {}. Got: {}",
                       ParserConsts::default_flag_prefix, flag_string);
}
} /* namespace Arguinator */
