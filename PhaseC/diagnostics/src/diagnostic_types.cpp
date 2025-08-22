#include <sstream>
#include <diagnostics/diagnostic_types.hpp>

namespace alpha
{
Suggestion::Suggestion(const std::string &text, const SourceLocation insert_after)
    : text(text), insert_after(insert_after)
{
    // Remove this assertion, in case there are useful multiline suggestions.
    DEBUG_SMART_ASSERT(
        text.find('\n') == std::string::npos &&
        "Suggestion should be contained inside a single line"
    );
}

u32
Suggestion::compute_printing_span() const
{
    constexpr u32 k_line_offset = 1; // non-empty text always has one more line than '\n' count

    if (text.empty())
        return 0;

    const auto newline_count = std::count(text.begin(), text.end(), '\n');
    DEBUG_SMART_ASSERT(
        newline_count >= 0,
        newline_count < std::numeric_limits<u32>::max() - k_line_offset
    );
    return static_cast<u32>(newline_count) + k_line_offset;
}

Issue::Issue(const Type type, std::string description, const SourceLocation loc)
    : type(type),
      desc(std::move(description)),
      loc(loc),
      suggestion(std::nullopt) {}

Issue::Issue(
    const Type type,
    std::string description,
    const SourceLocation loc,
    Suggestion suggestion)
    : type(type),
      desc(std::move(description)),
      loc(loc),
      suggestion(std::move(suggestion)) {}

u32
Issue::line(const LocationTracker &loc_tracker) const { return loc_tracker.find_first_line(loc); }

u32
Issue::column(const LocationTracker &loc_tracker) const
{
    return loc_tracker.find_first_column(loc);
}

Issue::RenderingSpan
Issue::compute_printing_span(const LocationTracker &loc_tracker) const
{
    // Start with the span of the issue itself.
    RenderingSpan result{
        .start_line = loc_tracker.find_first_line(loc),
        .end_line = loc_tracker.find_last_line(loc)
    };

    // Expand span if the suggestion lies outside the issue location.
    if (suggestion.has_value())
    {
        const u32 suggestion_start_line = loc_tracker.find_first_line(suggestion->insert_after);
        const u32 suggestion_end_line = loc_tracker.find_last_line(suggestion->insert_after);

        if (suggestion_start_line < result.start_line)
            result.start_line = suggestion_start_line;
        if (suggestion_end_line > result.end_line)
            result.end_line = suggestion_end_line;
    }

    return result;
}

const char *to_string(const Issue::Type type) noexcept
{
    using IT = Issue::Type;
    switch (type)
    {
    case IT::NOTE: return "note";
    case IT::WARNING: return "warning";
    case IT::SOFT_ERROR:
    case IT::HARD_ERROR: return "error";
    case IT::FATAL_ERROR: return "fatal-error";
    default: UNREACHABLE("Unknown Issue Type!");
    }
}

Diagnostic::Diagnostic(
    Issue &&primary,
    std::list<Note> &&note_list)
    : primary(std::move(primary)),
      note_list(std::move(note_list)) {}
} // namespace alpha
