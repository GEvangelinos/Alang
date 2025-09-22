#ifndef ARGUINATOR_HPP
#define ARGUINATOR_HPP

#include <cstddef>   // for size_t
#include <functional>
#include <map>       // for map
#include <stdexcept> // for runtime_error
#include <string>    // for basic_string, string
#include <vector>    // for vector

namespace arguinator
{
class Flag;     // IWYU pragma: keep
class CLIHelp;  // IWYU pragma: keep
class CLIError; // IWYU pragma: keep

class Parser
{
public:
    Parser(int argc, const char *const *argv, const std::string &description);
    Parser() = delete;

    Flag &set_flag(const std::string &flag_name);
    void parse_flags();
    [[nodiscard]] std::size_t count(const std::string &flag_name) const;
    [[nodiscard]] bool found(const std::string &flag_name) const;

    [[nodiscard]]
    const Flag &operator[](const std::string &flag_name) const;

private:
    std::size_t argc_;
    const char *const *const argv_;
    const std::string description_;
    std::map<std::string, Flag> flag_map_;

    void parse_flags_impl();
    [[nodiscard]] std::string generate_help_text() const;
    [[noreturn]] void display_help_page();
}; /* class Parser */

class Flag
{
    friend class Parser;

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
    [[nodiscard]] std::size_t get_arity() const noexcept;
    [[nodiscard]] const std::string &get_help_text() const noexcept;
    [[nodiscard]] bool is_required() const noexcept;
    [[nodiscard]] bool is_provided() const noexcept;
    [[nodiscard]] const std::string &get_name() const noexcept;
    [[nodiscard]] const std::vector<std::string> &get_inputs() const noexcept;

    /* By default, it returns first input, helpful in argument with arity 1. */
    [[nodiscard]] const std::string &get_input(std::size_t input_field = 1) const;

private:
    /* Private constructor */
    explicit Flag(const std::string &name); /* Only usable via set_flag() */

    /* Internal modifiers */
    void set_provided() noexcept;

    const std::string name_;
    std::size_t arity_; /* Number of required inputs (e.g. --rgb 255 255 0) */
    std::string help_text_;
    bool required_;
    bool provided_;
    std::vector<std::string> inputs_; /* Values passed to flag */
};

// Used to signal arguinator's user that Help-Text was displayed... It should probably exit().
class CLIHelp {};

class CLIError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};
} /* namespace Arguinator */

#endif /* ARGUINATOR_HPP */
