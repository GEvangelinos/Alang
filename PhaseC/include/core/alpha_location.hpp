#ifndef ALPHA_LOCATION_HPP
#define ALPHA_LOCATION_HPP

#include <vector>               // for vector
#include "core/alpha_types.hpp" // for u32

namespace Alpha
{
        // Classes defined here:
        struct Location;        // IWYU pragma: keep
        struct LineRange;      // IWYU pragma: keep
        class LocationTracker; // IWYU pragma: keep

        struct Location
        {
                u32 first_index;
                u32 last_index;
        };

        struct LineRange
        {
                const u32 first_line;
                const u32 last_line;
        };

        class LocationTracker
        {
        public:
                LocationTracker(u32 max_valid_index);

                void append_line(u32 start_index);
                u32 find_first_line(Location location) const;
                u32 find_last_line(Location location) const;
                u32 find_symbol_line(Location location) const; // Symbol is always defined in 1 line.
                u32 find_index_of_line(u32 line) const;
                u32 find_first_column(Location location) const;
                LineRange find_lines(u32 first_index, u32 last_index) const;
                LineRange find_lines(Location location) const;

        private:
                const u32 max_valid_index_;
                std::vector<u32> line_start_indices_;

                u32 find_line(u32 index) const;
        };
} // namespace Alpha
#endif // ALPHA_LOCATION_HPP
