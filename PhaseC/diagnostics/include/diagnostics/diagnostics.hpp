#ifndef ALPHA_ISSUE_HPP
#define ALPHA_ISSUE_HPP

#include <list>                    // for list
#include <string>                  // for string, basic_string
#include <string_view>             // for string_view
#include "core/source_location.hpp" // for SourceLocation, SourceLocationTracker

namespace Alpha
{
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

    friend class DiagnosticEngine;
};
} // namespace Alpha
#endif // ALPHA_ISSUE_HPP
