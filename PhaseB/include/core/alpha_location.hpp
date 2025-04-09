#ifndef ALPHA_LOCATION_HPP
#define ALPHA_LOCATION_HPP

#include "core/alpha_types.hpp"

namespace Alpha
{
        struct CodeLocation
        {
        public:
                u32 first_index_;
                u32 last_index_;

                constexpr CodeLocation() noexcept
                    : first_index_(0),
                      last_index_(0) {}

                constexpr CodeLocation(u32 first_index, u32 last_index) noexcept
                    : first_index_(first_index),
                      last_index_(last_index) {}

                CodeLocation(const CodeLocation &other) noexcept = default;
                CodeLocation &operator=(const CodeLocation &other) noexcept = default;

                CodeLocation(CodeLocation &&other) noexcept = default;
                CodeLocation &operator=(CodeLocation &&other) noexcept = default;
                ~CodeLocation() noexcept = default;
        };
}

#endif /* ALPHA_LOCATION_HPP */
