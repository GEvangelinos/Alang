#include <sstream>
#include <diagnostics/diagnostic_types.hpp>

namespace alpha
{
Suggestion::Suggestion(const std::string_view desc, const SourceLocation insert_after)
    : desc(desc), insert_after(insert_after) {}

u32
Suggestion::compute_printing_span() const
{
    constexpr u32 k_line_offset = 1; // non-empty text always has one more line than '\n' count

    if (desc.empty())
        return 0;

    const auto newline_count = std::count(desc.begin(), desc.end(), '\n');
    DEBUG_SMART_ASSERT(
        newline_count >= 0,
        newline_count < std::numeric_limits<u32>::max() - k_line_offset
    );
    return static_cast<u32>(newline_count) + k_line_offset;
}

Issue::Issue(
    const Type type,
    std::string description,
    const SourceLocation loc,
    std::optional<Suggestion> suggestion,
    std::optional<std::vector<Highlight>> highlights)
    : type(type),
      desc(std::move(description)),
      loc(loc),
      suggestion(std::move(suggestion)),
      highlights(std::move(highlights))
{
    // Assert highlights underline correct code regions.
    if (!highlights.has_value())
        return;
    for (const Highlight &h : *highlights)
    {
        if (h.loc.begin < loc.begin)
            throw std::logic_error(ATTACH_CONTEXT("Highlight points before Issue's location"));
        if (h.loc.end > loc.end)
            throw std::logic_error(ATTACH_CONTEXT("Highlight points after  Issue's location"));
    }
}

SrcLineIdx
Issue::line(const LocationTracker &loc_tracker) const { return loc_tracker.find_first_line(loc); }

SrcColumnIdx
Issue::column(const LocationTracker &loc_tracker) const
{
    return loc_tracker.find_first_column(loc);
}

Issue::RenderingLineSpan
Issue::compute_rendering_span(const LocationTracker &loc_tracker) const
{
    // Start with the span of the issue itself.
    RenderingLineSpan result{
        .begin_line = loc_tracker.find_first_line(loc),
        .end_line = loc_tracker.find_last_line(loc)
    };

    auto adjust_to_fit_span = [&result, &loc_tracker](const SourceLocation loc_target)
    {
        const SrcLineIdx suggestion_start_line = loc_tracker.find_first_line(loc_target);
        const SrcLineIdx suggestion_end_line = loc_tracker.find_last_line(loc_target);

        if (suggestion_start_line < result.begin_line)
            result.begin_line = suggestion_start_line;
        if (suggestion_end_line > result.end_line)
            result.end_line = suggestion_end_line;
    };

    // Expand span if the suggestion lies outside the issue location.
    if (suggestion.has_value())
        adjust_to_fit_span(suggestion->insert_after);

    // Expand span if the highlights lie outside the rendering span.
    if (highlights.has_value())
        for (const Highlight &hl : *highlights)
            adjust_to_fit_span(hl.loc);

    DEBUG_SMART_ASSERT(result.begin_line <= result.end_line);
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
    const DiagnosticCode code,
    Issue &&primary,
    std::list<Note> &&note_list)
    : code(code),
      primary(std::move(primary)),
      note_list(std::move(note_list)) {}
} // namespace alpha
