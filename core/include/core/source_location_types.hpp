#ifndef SOURCE_LOCATION_TYPES_HPP
#define SOURCE_LOCATION_TYPES_HPP

#include "numeric_types.hpp"
#include "strong_type.hpp"

namespace alpha
{
struct SrcLineIdx : StrongType<SrcLineIdx, u32, 0> { using StrongType::StrongType; };
struct SrcColumnIdx : StrongType<SrcColumnIdx, u32, 0> { using StrongType::StrongType; };
struct SrcBuffIdx : StrongType<SrcBuffIdx, u32, 0> { using StrongType::StrongType; };
} // namespace alpha
#endif // SOURCE_LOCATION_TYPES_HPP
