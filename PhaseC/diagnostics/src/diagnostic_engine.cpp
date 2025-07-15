#include <algorithm>                // for count
#include <cstring>                  // for size_t, strchr, strlen
#include <sstream>                  // for basic_stringstream, basic_ostream
#include <stdexcept>
#include <utility>                  // for move
#include <diagnostics/diagnostic_engine.hpp>
#include <utils/debug_tools.hpp>
#include <utils/format_adapter.hpp>
#include "core/source_location.hpp"  // for SourceLocation, SourceLocationTracker
#include "utils/cli_color.h"        // for COLOR_ASCII_BOLD_DEFAULT, SGR_RESET
#include "utils/misc.hpp"
#include "utils/smart_assert.h" // for DEBUG_SMART_ASSERT

namespace
{
std::vector<std::string_view> extract_line_views(
    const char *const buffer, const std::size_t start_index, const std::size_t end_index)
{
    std::vector<std::string_view> lines;
    const char *start = buffer + start_index;
    const char *current = start;
    const char *end_target = buffer + end_index;

    while (*current)
    {
        const char *line_end = std::strchr(current, '\n');
        if (!line_end)
            line_end = current + std::strlen(current); // to '\0' if no newline

        lines.emplace_back(current, line_end - current); // view [current, line_end)

        if (end_target <= line_end)
            break; // stop if end_index is in this line

        current = *line_end == '\n' ? line_end + 1 : line_end;
    }
    return lines;
}

std::string expand_tabs(const std::string_view line, const int tab_width = 8)
{
    std::string result;
    result.reserve(line.size() + std::count(line.begin(), line.end(), '\t') * (tab_width - 1));

    Alpha::uf64 col = 0;
    for (const char ch: line)
    {
        if (ch == '\t')
        {
            const int spaces = tab_width - col % tab_width;
            result.append(spaces, ' ');
            col += spaces;
        }
        else
        {
            result += ch;
            ++col;
        }
    }
    return result;
}

int compute_visual_caret_offset(const std::string_view line, const Alpha::uf64 raw_offset,
                                const int tab_width = 8)
{
    Alpha::uf64 col = 0;
    for (Alpha::uf64 i = 0; i < raw_offset; ++i)
        col += line[i] == '\t' ? tab_width - col % tab_width : 1;
    return col;
}
} // namespace

namespace Alpha
{
Issue::Issue(const Type type, const std::string &description, const SourceLocation loc)
    : type(type), desc(description), loc(loc) {}

u32 Issue::line(const LocationTracker &lt) const { return lt.find_first_line(loc); }

u32 Issue::column(const LocationTracker &lt) const { return lt.find_first_column(loc); }

std::string_view Issue::type_to_string() const noexcept
{
    switch (type)
    {
    case Type::ERROR: return "issue";
    case Type::WARNING: return "warning";
    case Type::NOTE: return "note";
    default: UNREACHABLE("Unknown Issue Type!");
    }
}

std::string_view Issue::pretty_color() const noexcept
{
    switch (type)
    {
    case Type::ERROR: return COLOR_ASCII_BOLD_RED;
    case Type::WARNING: return COLOR_ASCII_BOLD_MAGENTA;
    case Type::NOTE: return COLOR_ASCII_BOLD_CYAN;
    default: UNREACHABLE("Unknown Issue Type!");
    }
}

std::string Diagnostic::make_pretty_diagnostic(const std::string &source_filename,
                                               const LocationTracker &lt,
                                               const char *input_buffer) const
{
    std::stringstream ss;
    ss << make_pretty_diagnostic_impl(source_filename, lt, input_buffer, primary);
    for (auto note: notes)
        ss << make_pretty_diagnostic_impl(source_filename, lt, input_buffer, note);
    return ss.str();
}

Diagnostic::Diagnostic(
    const Issue::Type type,
    const std::string &desc,
    const SourceLocation loc)
    : primary(type, desc, loc) {}

Diagnostic::Diagnostic(
    const Issue::Type type,
    const std::string &desc,
    const SourceLocation loc,
    std::list<Note> &&note_list_)
    : primary(type, desc, loc),
      notes(std::move(note_list_)) {}

// TODO: Fix.. its ugly AF
std::string Diagnostic::make_pretty_diagnostic_impl(const std::string &source_filename,
                                                    const LocationTracker &lt,
                                                    const char *input_buffer,
                                                    const Issue &issue)
{
    const u32 issue_line = issue.line(lt);
    const u32 issue_column = issue.column(lt);
    /* Error header: */
    std::stringstream ss;
    ss << COLOR_ASCII_BOLD_DEFAULT
            << FMT::format("{}:{}:{}: {}{}{}: {}\n", source_filename, issue_line, issue_column,
                           issue.pretty_color(), issue.type_to_string(), COLOR_ASCII_BOLD_DEFAULT,
                           issue.desc)
            << SGR_RESET;

    const auto line_views = extract_line_views(input_buffer, lt.find_index_of_line(issue_line),
                                               issue.loc.last_index);
    for (std::size_t i = 0; i < line_views.size(); i++)
    {
        constexpr u32 line_box_width = 8;
        std::string visual_line = expand_tabs(line_views[i]);
        ss << FMT::format("{:>{}} | {}\n", i != 0 ? "" : std::to_string(issue_line),
                          line_box_width, visual_line);
        if (i != 0) // Caret marking is only for first line.
            continue;

        DEBUG_SMART_ASSERT(issue.loc.last_index > issue.loc.first_index);

        const auto raw_caret_offset =
                issue.loc.first_index -
                lt.find_index_of_line(lt.find_first_line(issue.loc));
        const auto visual_caret_offset =
                compute_visual_caret_offset(line_views[i], raw_caret_offset);
        const auto highlight_length =
                issue.loc.last_index - issue.loc.first_index - 1;

        ss << FMT::format(
            "{} | {}{}^{}\n", std::string(line_box_width, ' '), // Spaces pre  |
            std::string(visual_caret_offset, ' '),              // spaces post | to move caret
            issue.pretty_color(), std::string(highlight_length, '~'));
        ss << SGR_RESET;
    }

    return ss.str();
}

void DiagnosticEngine::report(
    const Issue::Type type,
    const std::string &desc,
    const SourceLocation loc)
{
    store(std::unique_ptr<const Diagnostic>(new const Diagnostic(type, desc, loc)));
}

void DiagnosticEngine::report(
    const Issue::Type type,
    const std::string &desc,
    const SourceLocation loc,
    std::list<Note> &&note_list)
{
    store(std::unique_ptr<const Diagnostic>(
        new const Diagnostic(type, desc, loc, std::move(note_list))));
}

void DiagnosticEngine::store(std::unique_ptr<const Diagnostic> diagnostic)
{
    diagnostics_.push_back(std::move(diagnostic)); // Pass ownership to `diagnostics_` vector.
    const Diagnostic *const dptr = diagnostics_.back().get();
    switch (dptr->primary.type)
    {
    case Issue::Type::WARNING:
        warnings_.push_back(dptr);
        break;
    case Issue::Type::ERROR:
        errors_.push_back(dptr);
        break;
    case Issue::Type::FATAL_ERROR:
        fatals_.push_back(dptr);
        break;
    case Issue::Type::NOTE:
        throw std::logic_error(ATTACH_CONTEXT(
            "Issue::Type::NOTE is used to add auxiliary info. Should not be used as main Issue"));
    default: UNREACHABLE("Unknown Issue::Type.");
    }
}
} // namespace Alpha
