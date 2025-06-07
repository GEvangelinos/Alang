#ifndef ALPHA_ISSUE_HPP
#define ALPHA_ISSUE_HPP

#include <list>                    // for list
#include <memory>                  // for unique_ptr
#include <string>                  // for string, basic_string
#include <string_view>             // for string_view
#include <vector>                  // for vector
#include "alpha_basics.hpp"
#include "core/alpha_location.hpp" // for SourceLocation, SourceLocationTracker

namespace Alpha
{
// Classes defined here:
class Issue;   // IWYU pragma: keep
class CTIssue;      // IWYU pragma: keep
class Diagnostics; // IWYU pragma: keep
class Issue
{
public:
    enum class Type : u8
    {
        FATAL_ERROR,
        ERROR,
        WARNING,
        NOTE,
    };

    const Type type;
    const std::string description;
    const SourceLocation loc;

    Issue(Type type, const std::string &description, SourceLocation loc);

    [[nodiscard]] u32 line(const LocationTracker &lt) const;
    [[nodiscard]] u32 column(const LocationTracker &lt) const;
    [[nodiscard]] std::string_view type_to_string() const noexcept;
    [[nodiscard]] std::string_view pretty_color() const noexcept;
};

class Note : public Issue
{
public:
    Note(const std::string &description, const SourceLocation loc)
        : Issue(Type::NOTE, description, loc) {}
};

class CTIssue
{
public:
    const Issue issue;
    const std::list<Note> note_list;

    CTIssue() = delete;

    [[nodiscard]] std::string make_pretty_diagnostic(
        const std::string &source_filename,
        const LocationTracker &lt,
        const char *input_buffer) const;

private:
    CTIssue(
        Issue::Type type,
        const std::string &issue_desc,
        SourceLocation issue_loc);
    CTIssue(
        Issue::Type type,
        const std::string &issue_desc,
        SourceLocation issue_loc,
        std::list<Note> &&note_list_);

    [[nodiscard]] static std::string make_pretty_issue_impl(
        const std::string &source_filename,
        const LocationTracker &lt,
        const char *input_buffer,
        const Issue &issue);

    friend class Diagnostics;
};

class Diagnostics : private Immobile
{
public:
    Diagnostics() = default;

    void report(Issue::Type type,
                const std::string &issue_desc,
                SourceLocation issue_loc);
    void report(Issue::Type type,
                const std::string &issue_desc,
                SourceLocation issue_loc,
                std::list<Note> &&note_list_);

    [[nodiscard]] const std::vector<std::unique_ptr<const CTIssue>> &
    get_compile_time_issues() const noexcept { return issues_; }

    [[nodiscard]] bool contain_issues() const noexcept { return !issues_.empty(); }
    [[nodiscard]] bool contain_fatal_errors() const noexcept { return !fatal_errors_.empty(); }
    [[nodiscard]] bool contain_errors() const noexcept { return !errors_.empty(); }
    [[nodiscard]] bool contain_warnings() const noexcept { return !warnings_.empty(); }

private:
    std::vector<std::unique_ptr<const CTIssue>> issues_;
    std::vector<const CTIssue *> fatal_errors_;
    std::vector<const CTIssue *> errors_;
    std::vector<const CTIssue *> warnings_;

    void store(std::unique_ptr<const CTIssue> ct_issue);
};
} // namespace Alpha
#endif // ALPHA_ISSUE_HPP
