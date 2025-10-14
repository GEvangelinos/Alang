#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <vector>               // for vector
#include "core/basics.hpp"
#include "core/numeric_types.hpp" // for u32
#include "core/source_location_types.hpp"

namespace alpha
{
// Classes defined here:
struct SourceLocation; // IWYU pragma: keep
struct LineRange;      // IWYU pragma: keep
class LocationTracker; // IWYU pragma: keep

struct SourceLocationRaw
{
    SrcBufferIdx::UnderlyingType begin;
    SrcBufferIdx::UnderlyingType end;
};

struct SourceLocation
{
    SrcBufferIdx begin; // Inclusive
    SrcBufferIdx end;   // Exclusive

    constexpr SourceLocation();
    constexpr SourceLocation(SrcBufferIdx begin, SrcBufferIdx end);
    constexpr SourceLocation(SourceLocationRaw raw_loc);

    [[nodiscard]] SourceLocationRaw to_raw();
    [[nodiscard]] bool operator==(SourceLocation rhs) const noexcept;
    [[nodiscard]] bool operator!=(SourceLocation rhs) const noexcept;
};

// ======================================================================================
// Core inline utilities for SourceLocation.
// These are used heavily throughout expression builders in the parser and kept inline
// to minimize call overhead in hot parsing paths.
// ======================================================================================
constexpr
SourceLocation::SourceLocation()
    : SourceLocation(
        SrcBufferIdx{SrcBufferIdx::none},
        SrcBufferIdx{SrcBufferIdx::none}
    ) {}

constexpr
SourceLocation::SourceLocation(SrcBufferIdx begin, SrcBufferIdx end)
    : begin(begin), end(end) {}

constexpr
SourceLocation::SourceLocation(const SourceLocationRaw raw_loc)
    : begin(SrcBufferIdx{raw_loc.begin}), end(SrcBufferIdx{raw_loc.end}) {}

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
        left.begin.value < right.begin.value,
        left.end.value < right.begin.value,
        left.begin.value < right.end.value && "Location overlap detected"
    );
    return SourceLocation{left.begin, right.end};
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
    const SrcLineIdx begin_line;
    const SrcLineIdx end_line;
};

class LocationTracker : private Immobile
{
public:
    explicit LocationTracker(std::size_t max_valid_index);

    void append_line(SrcBufferIdx linestart_index);
    [[nodiscard]] SrcLineIdx find_first_line(SourceLocation loc) const;
    [[nodiscard]] SrcLineIdx find_last_line(SourceLocation loc) const;
    [[nodiscard]] SrcLineIdx find_symbol_line(SourceLocation loc) const;
    [[nodiscard]] SrcBufferIdx find_index_of_line(SrcLineIdx line) const;
    [[nodiscard]] SrcColumnIdx find_first_column(SourceLocation loc) const;
    [[nodiscard]] LineRange find_lines(SrcBufferIdx begin_idx, SrcBufferIdx end_idx) const;
    [[nodiscard]] LineRange find_lines(SourceLocation loc) const;

private:
    const SrcBufferIdx max_valid_index_;
    std::vector<SrcBufferIdx> linestart_buffer_indices_;

    [[nodiscard]] SrcLineIdx find_line(SrcBufferIdx idx) const;
};
} // namespace alpha
#endif // SOURCE_LOCATION_HPP
