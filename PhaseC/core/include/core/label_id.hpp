#ifndef LABEL_ID_HPP
#define LABEL_ID_HPP

#include "numeric_types.hpp"
#include "strong_type.hpp"

namespace alpha
{
struct LabelID : StrongType<LabelID, u32>
{
    using StrongType::StrongType;
};
} // namespace alpha
#endif // LABEL_ID_HPP
