#ifndef KONSTANTS_HPP
#define KONSTANTS_HPP
#include "core/numeric_types.hpp"

namespace Alpha
{
using LabelID = u32;
using AlphaInt = i64;
using AlphaFloat = f64;

static constexpr LabelID k_no_label = 0;
static constexpr LabelID k_first_label = 1;

static_assert(k_first_label > k_no_label);
} // namespace Alpha
#endif // KONSTANTS_HPP
