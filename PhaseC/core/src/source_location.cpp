#include "core/source_location.hpp"
#include <algorithm>                // for is_sorted, upper_bound
#include <cstddef>                  // for ptrdiff_t
#include <iterator>                 // for distance
#include <stdexcept>                // for logic_error
#include "core/konstants.hpp" // for k_no_line
#include "utils/smart_assert.h"     // for DEBUG_SMART_ASSERT

namespace alpha
{
SourceLocation
merge(const SourceLocation left, const SourceLocation right)
{
    DEBUG_SMART_ASSERT(
        left.first_index < right.first_index,
        left.last_index < right.first_index,
        left.first_index < right.last_index,
    );
    return {
        .first_index = left.first_index,
        .last_index = right.last_index
    };
}

LocationTracker::LocationTracker(const u32 max_valid_index)
    : max_valid_index_(max_valid_index)
{
    line_start_indices_.push_back(k_no_line); // Pushes virtual line 0.
}

void
LocationTracker::append_line(const u32 start_index) { line_start_indices_.push_back(start_index); }

u32
LocationTracker::find_first_line(const SourceLocation location) const
{
    if (location == k_no_loc)
        return k_no_line;
    return find_line(location.first_index);
}

u32
LocationTracker::find_last_line(const SourceLocation location) const
{
    if (location == k_no_loc)
        return k_no_line;
    return find_line(location.last_index);
}

u32
LocationTracker::find_symbol_line(const SourceLocation location) const
{
    if (location == k_no_loc)
        return k_no_line;

    auto [begin_line, end_line] = find_lines(location);

    if (begin_line != end_line)
        throw std::logic_error(ATTACH_CONTEXT(
            "BUG: Symbol spans multiple lines."
            "Symbol must be defined on a single line."));
    return begin_line;
}

u32
LocationTracker::find_index_of_line(const u32 line) const
{
    if (line == k_no_line)
        throw std::logic_error(ATTACH_CONTEXT(FMT::format(
            "BUG: LocationTracker was asked to find index of k_no_line = `{}`)", k_no_line)));
    DEBUG_SMART_ASSERT(line <= line_start_indices_.size());
    return line_start_indices_[line - 1]; // -1 as line starts at pos 0.
}

u32
LocationTracker::find_first_column(const SourceLocation location) const
{
    if (location == k_no_loc)
        throw std::logic_error(ATTACH_CONTEXT(
            "BUG: LocationTracker was asked to find column of k_no_loc"));

    const u32 starting_line = find_first_line(location);
    // TODO: is this problematic? (the assertion, what do we even try to assert).
    DEBUG_SMART_ASSERT(starting_line <= line_start_indices_.size());
    // -1 as line starts at pos 0.
    const u32 index_at_starting_line = line_start_indices_[starting_line - 1];
    DEBUG_SMART_ASSERT(location.first_index >= index_at_starting_line);

    // +1 to convert index offset to columns.
    const u32 starting_column = location.first_index - index_at_starting_line + 1;
    return starting_column;
}

LineRange
LocationTracker::find_lines(const u32 first_index, const u32 last_index) const
{
    if (first_index == 0 && last_index == 0)
        // LIBFUNCs -- Only libfuncs are defined at Location{0,0}
        return {k_no_line, k_no_line};
    if (first_index == last_index)
        throw std::logic_error(ATTACH_CONTEXT(
            "BUG: Location with zero length. Start and end index are equal."));
    return {find_line(first_index), find_line(last_index)};
}

LineRange
LocationTracker::find_lines(const SourceLocation location) const
{
    return find_lines(location.first_index, location.last_index);
}

u32
LocationTracker::find_line(const u32 index) const
{
    DEBUG_SMART_ASSERT(std::is_sorted(line_start_indices_.begin(), line_start_indices_.end()));

    if (index > max_valid_index_)
        throw std::logic_error(ATTACH_CONTEXT(
            "BUG: LocationTracker received out-of-bounds index."));

    const auto it = std::upper_bound(line_start_indices_.begin(), line_start_indices_.end(), index);

    const std::ptrdiff_t line = std::distance(line_start_indices_.begin(), it);
    if (line < 0 || line > static_cast<std::ptrdiff_t>(line_start_indices_.size()))
        throw std::logic_error(ATTACH_CONTEXT("BUG: Invalid computed line index."));
    return static_cast<u32>(line);
}
} // namespace alpha
