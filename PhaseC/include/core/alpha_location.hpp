#ifndef ALPHA_LOCATION_HPP
#define ALPHA_LOCATION_HPP

#include <vector>               // for vector
#include "core/alpha_types.hpp" // for u32
#include "utils/misc.hpp"

namespace Alpha
{
// Classes defined here:
struct SourceLocation;       // IWYU pragma: keep
struct LineRange;      // IWYU pragma: keep
class LocationTracker; // IWYU pragma: keep

struct SourceLocation
{
        u32 first_index;
        u32 last_index;

        bool operator==(const SourceLocation &) const noexcept = default;
        bool operator!=(const SourceLocation &) const noexcept = default;
};

struct LineRange
{
        const u32 first_line;
        const u32 last_line;
};

class LocationTracker : private Immobile
{
public:
        LocationTracker(u32 max_valid_index);

        void append_line(u32 start_index);
        [[nodiscard]] u32 find_first_line(SourceLocation location) const;
        [[nodiscard]] u32 find_last_line(SourceLocation location) const;
        [[nodiscard]] u32
        find_symbol_line(SourceLocation location) const; // Symbol is always defined in 1 line.
        [[nodiscard]] u32 find_index_of_line(u32 line) const;
        [[nodiscard]] u32 find_first_column(SourceLocation location) const;
        [[nodiscard]] LineRange find_lines(u32 first_index, u32 last_index) const;
        [[nodiscard]] LineRange find_lines(SourceLocation location) const;

private:
        const u32 max_valid_index_;
        std::vector<u32> line_start_indices_;

        [[nodiscard]] u32 find_line(u32 index) const;
};
} // namespace Alpha
#endif // ALPHA_LOCATION_HPP
