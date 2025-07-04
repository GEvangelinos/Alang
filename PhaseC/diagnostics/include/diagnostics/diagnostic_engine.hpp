#ifndef DIAGNOSTIC_ENGINE_HPP
#define DIAGNOSTIC_ENGINE_HPP

#include <memory>
#include <core/basics.hpp>
#include <list>                    // for list
#include <string>                  // for string, basic_string
#include <string_view>             // for string_view
#include "core/source_location.hpp" // for SourceLocation, SourceLocationTracker
#include <diagnostics/diagnostic_reporter.gen.hpp>

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
    const std::string desc;
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
    Note(const std::string &desc, const SourceLocation loc)
        : Issue(Type::NOTE, desc, loc) {}
};

class Diagnostic
{
public:
    const Issue primary;
    const std::list<Note> notes;

    Diagnostic() = delete;

    [[nodiscard]] std::string make_pretty_diagnostic(
        const std::string &source_filename,
        const LocationTracker &lt,
        const char *input_buffer) const;

private:
    Diagnostic(
        Issue::Type type,
        const std::string &desc,
        SourceLocation loc);
    Diagnostic(
        Issue::Type type,
        const std::string &desc,
        SourceLocation loc,
        std::list<Note> &&note_list_);

    [[nodiscard]] static std::string make_pretty_diagnostic_impl(
        const std::string &source_filename,
        const LocationTracker &lt,
        const char *input_buffer,
        const Issue &issue);

    friend class DiagnosticEngine;
};

class DiagnosticEngine : private Immobile
{
public:
    DiagnosticReporter dr;

    DiagnosticEngine()
        : dr(this) {}

    [[nodiscard]] const std::vector<std::unique_ptr<const Diagnostic> > &
    get_compile_time_issues() const noexcept { return diagnostics_; }

    [[nodiscard]] bool contain_issues() const noexcept { return !diagnostics_.empty(); }
    [[nodiscard]] bool contain_fatal_errors() const noexcept { return !fatals_.empty(); }
    [[nodiscard]] bool contain_errors() const noexcept { return !errors_.empty(); }
    [[nodiscard]] bool contain_warnings() const noexcept { return !warnings_.empty(); }

private:
    std::vector<std::unique_ptr<const Diagnostic> > diagnostics_;
    std::vector<const Diagnostic *> warnings_;
    std::vector<const Diagnostic *> errors_;
    std::vector<const Diagnostic *> fatals_;


    void report(Issue::Type type, const std::string &desc, SourceLocation loc);
    void report(Issue::Type type, const std::string &desc, SourceLocation loc,
                std::list<Note> &&note_list_);

    void store(std::unique_ptr<const Diagnostic> diagnostic);

    friend class DiagnosticReporter;
};
} // namespace Alpha

#endif // DIAGNOSTIC_ENGINE_HPP
