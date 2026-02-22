#include "core/source_location.hpp"
#include <algorithm>                // for is_sorted, upper_bound
#include <cstddef>                  // for ptrdiff_t
#include <iterator>                 // for distance
#include <stdexcept>                // for logic_error
#include "core/konstants.hpp" // for k_no_line
#include "support/smart_assert.h"     // for DEBUG_SMART_ASSERT

namespace alpha
{
SrcBuffIdx sizeT_to_srcBufferIdx(const std::size_t num)
{
    if (!support::is_in_numeric_range<SrcBuffIdx::UnderlyingType>(num))
        throw std::length_error(ATTACH_CONTEXT("`num` value can not be stored `SrcBufferIdx`"));
    return SrcBuffIdx{static_cast<SrcBuffIdx::UnderlyingType>(num)};
}

LocationTracker::LocationTracker(const std::size_t max_valid_index)
    : max_valid_index_(sizeT_to_srcBufferIdx(max_valid_index))
{
    linestart_buffer_indices_.emplace_back(SrcBuffIdx::none); // Sets virtual line 0 at idx 0.
}

void
LocationTracker::append_line(const SrcBuffIdx linestart_index)
{
    DEBUG_SMART_ASSERT(
        !lines_frozen &&
        "Lines are frozen, meaning scanning of the source file should have ended."
        "Therefore there should be no extra lines to add."
    );

    linestart_buffer_indices_.push_back(linestart_index);
}

SrcLineIdx
LocationTracker::find_first_line(const SourceLocation loc) const
{
    if (loc == k_no_loc)
        return SrcLineIdx(SrcLineIdx::none);
    return find_line(loc.begin);
}

SrcLineIdx
LocationTracker::find_last_line(const SourceLocation loc) const
{
    if (loc == k_no_loc)
        return SrcLineIdx{SrcLineIdx::none};
    return find_line(loc.end);
}

SrcLineIdx
LocationTracker::find_symbol_line(const SourceLocation loc) const
{
    if (loc == k_no_loc)
        return SrcLineIdx{SrcLineIdx::none};
    return find_lines(loc).begin_line;
}

SrcBuffIdx
LocationTracker::find_index_of_line(const SrcLineIdx line) const
{
    if (line.value == SrcLineIdx::none)
        throw std::logic_error(ATTACH_CONTEXT(FMT::format(
            "BUG: LocationTracker was asked to find index of k_no_line = `{}`)",
            SrcLineIdx::none
        )));
    DEBUG_SMART_ASSERT(line.value <= linestart_buffer_indices_.size());
    return linestart_buffer_indices_[line.value - 1]; // -1 as line starts at pos 0.
}

SrcColumnIdx
LocationTracker::find_first_column(const SourceLocation loc) const
{
    if (loc == k_no_loc)
        throw std::logic_error(ATTACH_CONTEXT(
            "BUG: LocationTracker was asked to find column of k_no_loc"));

    const SrcLineIdx starting_line = find_first_line(loc);
    DEBUG_SMART_ASSERT(starting_line.value <= linestart_buffer_indices_.size());
    // -1 as line starts at pos 0.
    const SrcBuffIdx index_at_starting_line = linestart_buffer_indices_[starting_line.value - 1];
    DEBUG_SMART_ASSERT(loc.begin >= index_at_starting_line);

    // +1 to convert index offset to columns.
    const SrcColumnIdx starting_column{loc.begin.value - index_at_starting_line.value + 1};
    return starting_column;
}

/**
 * @brief Map a byte-index range to source line numbers.
 *
 * (0,0) is a sentinel for LIBFUNCs → returns {k_no_line, k_no_line}.
 * Zero-width ranges (first_index == last_index) are valid; both ends map to the same line.
 * No normalization is performed; the caller must ensure
 */
LineRange
LocationTracker::find_lines(const SrcBuffIdx begin_idx, const SrcBuffIdx end_idx) const
{
    // LIBFUNCs -- Only libfuncs are defined at Location{0,0}
    if (begin_idx.value == SrcBuffIdx::none && end_idx.value == SrcBuffIdx::none)
        return {
            .begin_line = SrcLineIdx{SrcLineIdx::none},
            .end_line = SrcLineIdx{SrcLineIdx::none}
        };

    return {find_line(begin_idx), find_line(end_idx)};
}

LineRange
LocationTracker::find_lines(const SourceLocation loc) const
{
    return find_lines(loc.begin, loc.end);
}

bool
LocationTracker::is_virtual_line(const SrcLineIdx line) const noexcept
{
    DEBUG_SMART_ASSERT(
        lines_frozen &&
        "Querying if a line is virtual is only possible after lines are frozen. "
        "(aka after whole source file is scanned)"
    );
    DEBUG_SMART_ASSERT(!linestart_buffer_indices_.empty() && "There must be at least phony line 0");
    const auto current_existing_line_count = linestart_buffer_indices_.size() - 1;
    return line.value == SrcLineIdx::none || line.value > current_existing_line_count;
}

SrcLineIdx
LocationTracker::find_line(const SrcBuffIdx idx) const
{
    DEBUG_SMART_ASSERT(
        std::is_sorted(linestart_buffer_indices_.begin(), linestart_buffer_indices_.end())
    );

    if (idx.value > max_valid_index_.value)
        throw std::logic_error(ATTACH_CONTEXT(
            "BUG: LocationTracker received out-of-bounds index."));

    const auto it = std::upper_bound(
        linestart_buffer_indices_.begin(),
        linestart_buffer_indices_.end(),
        idx
    );

    DEBUG_SMART_ASSERT(!linestart_buffer_indices_.empty() && "There must be at least phony line 0");
    const std::ptrdiff_t lineno = std::distance(linestart_buffer_indices_.begin(), it);

    if (lineno < 0 || lineno > static_cast<std::ptrdiff_t>(linestart_buffer_indices_.size()))
        throw std::logic_error(ATTACH_CONTEXT("BUG: Invalid computed line index."));

    DEBUG_SMART_ASSERT(support::is_in_numeric_range<SrcLineIdx::UnderlyingType>(lineno));
    return SrcLineIdx{static_cast<SrcLineIdx::UnderlyingType>(lineno)};
}
} // namespace alpha
