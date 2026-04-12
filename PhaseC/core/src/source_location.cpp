#include "core/source_location.hpp"
#include <algorithm>                // for is_sorted, upper_bound
#include <cstddef>                  // for ptrdiff_t
#include <iterator>                 // for distance
#include <stdexcept>                // for logic_error
#include "support/smart_assert.h"     // for DMASSERT

namespace alpha
{
[[nodiscard, deprecated]] SrcBuffIdx
sizeT_to_srcBufferIdx(const std::size_t num)
{
    if (!support::is_in_numeric_range<SrcBuffIdx::UnderlyingType>(num))
        throw std::length_error(ATTACH_CONTEXT("`num` value can not be stored `SrcBufferIdx`"));
    return SrcBuffIdx{static_cast<SrcBuffIdx::UnderlyingType>(num)};
}

bool
SourceLocation::is_valid() const noexcept
{
    return begin < end ||
           (begin == SrcBuffIdx::none() && end == SrcBuffIdx::none()) ||
           (begin == SourceLocation::k_eof_begin_ && end == SourceLocation::k_eof_end_);
}

} // namespace alpha
