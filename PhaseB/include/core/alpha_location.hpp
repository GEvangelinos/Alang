#ifndef ALPHA_LOCATION_HPP
#define ALPHA_LOCATION_HPP

#include "core/alpha_types.hpp"

namespace Alpha
{
        struct Location
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
}

#endif /* ALPHA_LOCATION_HPP */
