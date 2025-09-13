#ifndef DIAGNOSTIC_TYPES_HPP
#define DIAGNOSTIC_TYPES_HPP
#include <list>
#include <optional>
#include <string>
#include <memory>

#include "core/numeric_types.hpp"
#include "core/source_location.hpp"
#include "diagnostics/diagnostic_codes.gen.hpp"

namespace alpha
{
struct Suggestion
{
    const std::string text;
    const SourceLocation insert_after;

    Suggestion() = delete;
    Suggestion(std::string_view text, SourceLocation insert_after);

    [[nodiscard]] u32 compute_printing_span() const;
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

    struct RenderingSpan
    {
        u32 start_line;
        u32 end_line;
    };

    const Type type;
    const std::string desc;
    const SourceLocation loc;
    std::optional<Suggestion> suggestion;

    Issue(
        Type type,
        std::string description,
        SourceLocation loc,
        std::optional<Suggestion> suggestion = std::nullopt);

    [[nodiscard]] u32 line(const LocationTracker &loc_tracker) const;
    [[nodiscard]] u32 column(const LocationTracker &loc_tracker) const;

    [[nodiscard]] RenderingSpan compute_printing_span(const LocationTracker &loc_tracker) const;
};

[[nodiscard]] const char *to_string(Issue::Type type) noexcept;

class Note final : public Issue
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
    const DiagnosticCode code;
    const Issue primary;
    const std::list<Note> note_list;

    Diagnostic() = delete;

private:
    Diagnostic(
        DiagnosticCode code,
        Issue &&primary,
        std::list<Note> &&note_list = std::list<Note>());
};
} // namespace alpha

#endif // DIAGNOSTIC_TYPES_HPP
