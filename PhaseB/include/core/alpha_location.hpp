#ifndef ALPHA_LOCATION_HPP
#define ALPHA_LOCATION_HPP

#include "core/alpha_types.hpp"
#include <vector>
namespace Alpha
{
        class Location
        {
        public:
                u32 first_index_;
                u32 last_index_;

                constexpr Location() noexcept
                    : first_index_(0),
                      last_index_(0) {}

                constexpr Location(u32 first_index, u32 last_index) noexcept
                    : first_index_(first_index),
                      last_index_(last_index) {}

                Location(const Location &other) noexcept = default;
                Location &operator=(const Location &other) noexcept = default;

                Location(Location &&other) noexcept = default;
                Location &operator=(Location &&other) noexcept = default;
                ~Location() noexcept = default;
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
                void append_line(u32 start_index) { line_start_indices_.push_back(start_index); }

                LineRange find_lines(u32 first_index, u32 last_index) const;
                LineRange find_lines(Location location) const;
                u32 find_symbol_line(Location location) const; // Symbol is always defined in 1 line.
                u32 find_first_line(Location location) const;
                u32 find_last_line(Location location) const;
                u32 find_first_column(Location location) const;
                u32 find_index_of_line(u32 line) const;

        private:
                std::vector<u32> line_start_indices_;
                const u32 max_valid_index_;

                u32 find_line(u32 index) const;
        };
}

#endif /* ALPHA_LOCATION_HPP */
