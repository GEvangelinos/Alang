#include "core/alpha_location.hpp"
#include "misc/sanity_assert.h"
#include <algorithm>
#include <stdexcept>
#include "core/alpha_konstants.hpp"

namespace Alpha
{
        LocationTracker::LocationTracker()
        {
                line_start_indices_.push_back(k_no_line); // Pushes virtual line 0.
        }

        u32 LocationTracker::find_line(u32 index) const
        {
                SANITY_ASSERT_TRUE(std::is_sorted(line_start_indices_.begin(), line_start_indices_.end()));

                if (index > max_valid_index_)
                        throw std::logic_error("Bug: LocationTracker received out-of-bounds index.");

                auto it = std::upper_bound(line_start_indices_.begin(), line_start_indices_.end(), index);

                std::ptrdiff_t line = std::distance(line_start_indices_.begin(), it);
                if (line < 0 || line > static_cast<std::ptrdiff_t>(line_start_indices_.size()))
                        throw std::logic_error("Bug:Invalid computed line index.");
                return static_cast<u32>(line);
        }

        LineRange LocationTracker::find_lines(u32 first_index, u32 last_index) const
        {
                if (first_index == 0 && last_index == 0) // LIBFUNCs -- Only libfuncs are defined at Location{0,0}
                        return {k_no_line, k_no_line};
                if (first_index == last_index) // Nothing can begin and end at same index (size == 0).
                        throw std::logic_error("Bug: Location with zero length. Start and end index must differ.");
                return {find_line(first_index), find_line(last_index)};
        }

        LineRange LocationTracker::find_lines(Location location) const
        {
                return find_lines(location.first_index_, location.last_index_);
        }

        u32 LocationTracker::find_symbol_line(Location location) const
        {
               auto [begin_line, end_line] = find_lines(location);

                if (begin_line != end_line)
                        throw std::logic_error("Bug: Symbol spans multiple lines. Symbol must be defined on a single line.");
                return begin_line;
        }
}