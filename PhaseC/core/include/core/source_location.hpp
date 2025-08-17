#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <vector>               // for vector
#include "core/basics.hpp"
#include "core/numeric_types.hpp" // for u32

namespace alpha
{
// Classes defined here:
struct SourceLocation; // IWYU pragma: keep
struct LineRange;      // IWYU pragma: keep
class LocationTracker; // IWYU pragma: keep

struct SourceLocation
{
    u32 first_index;
    u32 last_index;

    bool operator==(const SourceLocation &) const noexcept = default;
    bool operator!=(const SourceLocation &) const noexcept = default;
};

// Promoted to free function, instead of static inside the SourceLocation, so we can enable Argument-Dependent-Lookup
SourceLocation merge(SourceLocation left, SourceLocation right);

struct BlockSourceLocation
{
    SourceLocation begin;
    SourceLocation end;
};

struct LineRange
{
    const u32 first_line;
    const u32 last_line;
};

class LocationTracker : private Immobile
{
public:
    explicit LocationTracker(u32 max_valid_index);

    void append_line(u32 start_index);
    [[nodiscard]] u32 find_first_line(SourceLocation location) const;
    [[nodiscard]] u32 find_last_line(SourceLocation location) const;
    [[nodiscard]] u32 find_symbol_line(SourceLocation location) const;
    [[nodiscard]] u32 find_index_of_line(u32 line) const;
    [[nodiscard]] u32 find_first_column(SourceLocation location) const;
    [[nodiscard]] LineRange find_lines(u32 first_index, u32 last_index) const;
    [[nodiscard]] LineRange find_lines(SourceLocation location) const;

private:
    const u32 max_valid_index_;
    std::vector<u32> line_start_indices_;

    [[nodiscard]] u32 find_line(u32 index) const;
};
} // namespace alpha
#endif // SOURCE_LOCATION_HPP
