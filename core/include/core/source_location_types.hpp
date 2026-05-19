#ifndef SOURCE_LOCATION_TYPES_HPP
#define SOURCE_LOCATION_TYPES_HPP

#include "numeric_types.hpp"
#include "strong_type.hpp"

namespace alpha
{
#define MAKE_STRONG_TYPE(NAME, TYPE, UNDERLYING_NONE_VALUE) \
    struct NAME : StrongType<NAME, TYPE, UNDERLYING_NONE_VALUE> { using StrongType::StrongType; }


MAKE_STRONG_TYPE(SrcLineIdx, u32, 0);
MAKE_STRONG_TYPE(SrcColumnIdx, u32, 0);
MAKE_STRONG_TYPE(SrcBuffIdx, u32, 0);
#undef MAKE_STRONG_TYPE
} // namespace alpha
#endif // SOURCE_LOCATION_TYPES_HPP
