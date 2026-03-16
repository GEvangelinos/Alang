#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <vector>               // for vector

#include "source_location.hpp"
#include "source_location.hpp"
#include "source_location.hpp"
#include "source_location.hpp"
#include "source_location.hpp"
#include "core/basics.hpp"
#include "core/numeric_types.hpp" // for u32
#include "core/source_location_types.hpp"

namespace alpha
{
// Classes defined here:
class SourceLocation;  // IWYU pragma: keep
struct LineRange;      // IWYU pragma: keep
class LocationTracker; // IWYU pragma: keep

struct SourceLocationRaw
{
    SrcBuffIdx::UnderlyingType begin;
    SrcBuffIdx::UnderlyingType end;
};

class SourceLocation
{
public:
    SrcBuffIdx begin = SrcBuffIdx::none(); // Inclusive
    SrcBuffIdx end = SrcBuffIdx::none();   // Exclusive

    constexpr SourceLocation() = default;
    constexpr SourceLocation(SrcBuffIdx begin, SrcBuffIdx end);
    constexpr SourceLocation(SourceLocationRaw raw_loc);

    [[nodiscard]] SourceLocationRaw to_raw();
    [[nodiscard]] bool operator==(SourceLocation rhs) const noexcept;
    [[nodiscard]] bool operator!=(SourceLocation rhs) const noexcept;

    [[nodiscard]] static constexpr SourceLocation none() noexcept;
    [[nodiscard]] static constexpr SourceLocation eof() noexcept;

private:
    static constexpr SrcBuffIdx k_eof_begin_ =
        SrcBuffIdx{std::numeric_limits<SrcBuffIdx::UnderlyingType>::max()};
    static constexpr SrcBuffIdx k_eof_end_ =
        SrcBuffIdx{std::numeric_limits<SrcBuffIdx::UnderlyingType>::max()};

    [[nodiscard]] bool is_valid() const noexcept;
};

// ======================================================================================
// Core inline utilities for SourceLocation.
// These are used heavily throughout expression builders in the parser and kept inline
// to minimize call overhead in hot parsing paths.
// ======================================================================================
constexpr
SourceLocation::SourceLocation(const SrcBuffIdx begin, const SrcBuffIdx end)
    : begin(begin), end(end) { DEBUG_SMART_ASSERT(is_valid()); }

constexpr
SourceLocation::SourceLocation(const SourceLocationRaw raw_loc)
    : begin(SrcBuffIdx{raw_loc.begin}), end(SrcBuffIdx{raw_loc.end})
{
    DEBUG_SMART_ASSERT(is_valid());
}

inline SourceLocationRaw
SourceLocation::to_raw() { return {.begin = begin.value, .end = end.value}; }

inline bool
SourceLocation::operator==(const SourceLocation rhs) const noexcept
{
    return this->begin.value == rhs.begin.value &&
           this->end.value == rhs.end.value;
}

inline bool
SourceLocation::operator!=(const SourceLocation rhs) const noexcept { return !(*this == rhs); }

// Promoted to free function, instead of static inside the SourceLocation, so we can enable Argument-Dependent-Lookup
inline SourceLocation
merge(const SourceLocation left, const SourceLocation right)
{
    DEBUG_SMART_ASSERT(
        left.begin < left.end,   // Verify correct SourceLocation
        right.begin < right.end, // Verify correct SourceLocation
        left.begin < right.begin,
        left.begin < right.end,
        left.end <= right.begin && "Location overlap detected"
    );
    return SourceLocation{left.begin, right.end};
}

constexpr SourceLocation
SourceLocation::none() noexcept { return SourceLocation{SrcBuffIdx::none(), SrcBuffIdx::none()}; }

constexpr SourceLocation
SourceLocation::eof() noexcept
{
    return SourceLocation{SourceLocation::k_eof_begin_, SourceLocation::k_eof_end_};
}

// ======================================================================================
// ======================================================================================

struct BlockSourceLocation
{
    SourceLocationRaw begin_raw_loc;
    SourceLocationRaw end_raw_loc;
};

struct LineRange
{
    const SrcLineIdx begin_line; // Inclusive
    const SrcLineIdx end_line;   // Inclusive
};

} // namespace alpha
#endif // SOURCE_LOCATION_HPP
