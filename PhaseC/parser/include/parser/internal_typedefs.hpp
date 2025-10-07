#ifndef INTERNAL_TYPEDEFS_HPP
#define INTERNAL_TYPEDEFS_HPP
#include <vector>
#include <core/numeric_types.hpp>

namespace alpha
{
using TempHandleID = u32;
using LabelID = u32;
using AlphaInt = i64;
using AlphaFloat = f64;

struct Expr;

using ExprList = std::vector<const Expr *>;
using ExprPair = std::pair<const Expr *, const Expr *>;
} // namespace alpha

#endif // INTERNAL_TYPEDEFS_HPP
