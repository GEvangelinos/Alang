#ifndef TEMP_HANDLE_ID_HPP
#define TEMP_HANDLE_ID_HPP

#include "numeric_types.hpp"
#include "strong_type.hpp"

namespace alpha
{
using TempHandleID = StrongType<struct TempHandleIDTag, u32>;
} // namespace alpha
#endif // TEMP_HANDLE_ID_HPP
