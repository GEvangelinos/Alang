#ifndef DIAGNOSTIC_ENGINE_HPP
#define DIAGNOSTIC_ENGINE_HPP

#include <functional>
#include <list>                    // for list
#include <memory>
#include <optional>
#include <string>                  // for string, basic_string
#include <string_view>             // for string_view
#include <core/basics.hpp>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include "core/source_location.hpp" // for SourceLocation, SourceLocationTracker

// TODO: which classes can I encapsulate here?
namespace alpha
{
struct Suggestion
{
    Suggestion() = delete;

    Suggestion(const std::string &text, const SourceLocation insert_after)
        : text(text), insert_after(insert_after) {}

    const std::string text;
    const SourceLocation insert_after;
};

class Issue
{
public:
    enum class Type : u8
    {
        NOTE, // Never emitted alone (always combined with warning or error)
        WARNING,
        SOFT_ERROR,
        HARD_ERROR,
        FATAL_ERROR,
    };

    const Type type;
    const std::string desc;
    const SourceLocation loc;
    std::optional<Suggestion> suggestion;

    Issue(Type type, std::string description, SourceLocation loc);
    Issue(Type type, std::string description, SourceLocation loc, Suggestion suggestion);

    [[nodiscard]] u32 line(const LocationTracker &lt) const;
    [[nodiscard]] u32 column(const LocationTracker &lt) const;
    [[nodiscard]] std::string_view type_to_string() const noexcept;
    [[nodiscard]] std::string_view pretty_color() const noexcept;
};

class Note : public Issue
{
public:
    Note(std::string desc, const SourceLocation loc)
        : Issue(Type::NOTE, std::move(desc), loc) {}

    Note(std::string desc, const SourceLocation loc, Suggestion suggestion)
        : Issue(Type::NOTE, std::move(desc), loc, std::move(suggestion)) {}
};

class Diagnostic
{
    friend class DiagnosticEngine;

public:
    const Issue primary;
    const std::list<Note> note_list;

    Diagnostic() = delete;

    [[nodiscard]] std::string make_pretty_diagnostic(
        const std::string &source_filename,
        const LocationTracker &lt,
        const char *input_buffer) const;

private:
    explicit Diagnostic(Issue &&primary, std::list<Note> &&note_list = std::list<Note>());

    [[nodiscard]] static std::string make_pretty_diagnostic_impl(
        const std::string &source_filename,
        const LocationTracker &lt,
        const char *input_buffer,
        const Issue &issue);
};

class DiagnosticEngine : private Immobile
{
    friend class DiagnosticReporter;

public:
    struct Policy
    {
        std::function<bool()> should_emit_diagnostic;     // query: should DE emit this diagnostic?
        std::function<void()> notify_max_errors_reached; // notify: maximum error limit reached.
        std::function<void()> notify_fatal_error;         // notify: a fatal error occurred.
        std::function<void()> notify_hard_error;          // notify: a hard error occurred.
    };

    DiagnosticReporter dr;

    explicit DiagnosticEngine(
        Policy &&policy, std::optional<std::size_t> max_errors = std::nullopt);

    void report(Issue primary, std::list<Note> &&note_list = std::list<Note>());

    // TODO: add an export function for all diagnostics (export the diagnostics vector) // or DETATCH method
    [[nodiscard]] const auto &get_compile_time_issues() const noexcept { return diagnostics_; }
    [[nodiscard]] bool has_issues() const noexcept { return !diagnostics_.empty(); }
    [[nodiscard]] bool has_warnings() const noexcept { return !warnings_.empty(); }
    [[nodiscard]] bool has_soft_errors() const noexcept { return !softs_.empty(); }
    [[nodiscard]] bool has_hard_errors() const noexcept { return !hards_.empty(); }
    [[nodiscard]] bool has_fatal_errors() const noexcept { return !fatals_.empty(); }

    [[nodiscard]] bool has_errors() const noexcept
    {
        return has_soft_errors() || has_hard_errors() || has_fatal_errors();
    }

    [[nodiscard]] std::size_t error_count() const noexcept
    {
        return softs_.size() + hards_.size() + fatals_.size();
    }

private:
    const Policy policy_;
    const std::optional<std::size_t> max_errors;
    std::vector<std::unique_ptr<const Diagnostic>> diagnostics_;
    std::vector<const Diagnostic *> warnings_;
    std::vector<const Diagnostic *> softs_;
    std::vector<const Diagnostic *> hards_;
    std::vector<const Diagnostic *> fatals_;

    void report(
        Issue::Type type,
        std::string desc,
        SourceLocation loc,
        std::list<Note> &&note_list = std::list<Note>());

    void emit(Issue &&primary, std::list<Note> &&note_list);
};
} // namespace alpha

#endif // DIAGNOSTIC_ENGINE_HPP
