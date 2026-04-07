#ifndef LABEL_ID_HPP
#define LABEL_ID_HPP

#include "numeric_types.hpp"
#include "strong_type.hpp"

namespace alpha
{
using LabelID = StrongType<struct LabelIDTag, u32>;
} // namespace alpha
#endif // LABEL_ID_HPP
