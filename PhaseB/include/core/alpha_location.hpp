#ifndef ALPHA_LOCATION_HPP
#define ALPHA_LOCATION_HPP

#include "core/alpha_types.hpp"

namespace Alpha
{
    /* Indicies are used to quickly track the location
     * of the parsed rule  inside the input buffer (char *).
     */
    struct Location
    {
        u32 first_line_;
        u32 first_column_;
        u32 first_index_;
        u32 last_line_;
        u32 last_column_;
        u32 last_index_;
    };

    struct SourceRange
    {
    public:
        u32 first_index_;
        u32 last_index_;

        constexpr SourceRange(u32 first_index, u32 last_index)
            : first_index_(first_index),
              last_index_(last_index) {}

        SourceRange(const Location &location)
            : first_index_(location.first_index_),
              last_index_(location.last_index_) {}
    };

}

#endif /* ALPHA_LOCATION_HPP */
