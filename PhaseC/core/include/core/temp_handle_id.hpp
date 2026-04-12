#ifndef TEMP_HANDLE_ID_HPP
#define TEMP_HANDLE_ID_HPP

#include "numeric_types.hpp"
#include "strong_type.hpp"

namespace alpha
{
struct TempHandleID : StrongType<TempHandleID, u32> { using StrongType::StrongType; };
} // namespace alpha
#endif // TEMP_HANDLE_ID_HPP
