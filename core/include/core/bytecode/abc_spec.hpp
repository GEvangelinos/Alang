#ifndef ABC_SPEC_HPP
#define ABC_SPEC_HPP

#include "core/numeric_types.hpp"

namespace alpha::abc::spec
{
using MagicT = u32;
using StrCntT = u32;
using StrLenT = u32;

constexpr MagicT k_magic_value = 0xDEC0FAA1;
} // namespace alpha::abc::spec
#endif // ABC_SPEC_HPP
