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
    SrcBuffIdx::UnderlyingType begin;
    SrcBuffIdx::UnderlyingType end;
};

struct SourceLocation
{
    SrcBuffIdx begin; // Inclusive
    SrcBuffIdx end;   // Exclusive

    constexpr SourceLocation();
    constexpr SourceLocation(SrcBuffIdx begin, SrcBuffIdx end);
    constexpr SourceLocation(SourceLocationRaw raw_loc);

    [[nodiscard]] SourceLocationRaw to_raw();
    [[nodiscard]] bool operator==(SourceLocation rhs) const noexcept;
    [[nodiscard]] bool operator!=(SourceLocation rhs) const noexcept;

    friend std::ostream& operator<<(std::ostream &os, const SourceLocation& self);
};

inline std::ostream&
    operator<<(std::ostream& os, const SourceLocation& self)
{
    os << "[" << self.begin.value << ", " << self.end.value << ")";
    return os;

}


// ======================================================================================
// Core inline utilities for SourceLocation.
// These are used heavily throughout expression builders in the parser and kept inline
// to minimize call overhead in hot parsing paths.
// ======================================================================================
constexpr
SourceLocation::SourceLocation()
    : SourceLocation(
        SrcBuffIdx{SrcBuffIdx::none},
        SrcBuffIdx{SrcBuffIdx::none}
    ) {}

constexpr
SourceLocation::SourceLocation(SrcBuffIdx begin, SrcBuffIdx end)
    : begin(begin), end(end)
{
    DEBUG_SMART_ASSERT(
        ( begin.value == SrcBuffIdx::none &&
            end.value == SrcBuffIdx::none
        )
        || begin < end
    );
}

constexpr
SourceLocation::SourceLocation(const SourceLocationRaw raw_loc)
    : begin(SrcBuffIdx{raw_loc.begin}), end(SrcBuffIdx{raw_loc.end})
{
    DEBUG_SMART_ASSERT(
        ( begin.value == SrcBuffIdx::none &&
            end.value == SrcBuffIdx::none
        )
        || begin < end
    );
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
    OnceFlag lines_frozen;

    explicit LocationTracker(std::size_t max_valid_index);

    void append_line(SrcBuffIdx linestart_index);
    [[nodiscard]] SrcLineIdx find_first_line(SourceLocation loc) const;
    [[nodiscard]] SrcLineIdx find_last_line(SourceLocation loc) const;
    [[nodiscard]] SrcLineIdx find_symbol_line(SourceLocation loc) const;
    [[nodiscard]] SrcBuffIdx find_index_of_line(SrcLineIdx line) const;
    [[nodiscard]] SrcColumnIdx find_first_column(SourceLocation loc) const;
    [[nodiscard]] LineRange find_lines(SrcBuffIdx begin_idx, SrcBuffIdx end_idx) const;
    [[nodiscard]] LineRange find_lines(SourceLocation loc) const;
    [[nodiscard]] bool is_virtual_line(SrcLineIdx line) const noexcept;

private:
    const SrcBuffIdx max_valid_index_;
    std::vector<SrcBuffIdx> linestart_buffer_indices_;

    [[nodiscard]] SrcLineIdx find_line(SrcBuffIdx idx) const;
};
} // namespace alpha
#endif // SOURCE_LOCATION_HPP
