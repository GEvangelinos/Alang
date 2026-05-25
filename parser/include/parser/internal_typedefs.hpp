#ifndef INTERNAL_TYPEDEFS_HPP
#define INTERNAL_TYPEDEFS_HPP

#include <vector>

namespace alpha
{
struct Expr;

using ExprList = std::vector<const Expr *>;
using ExprPair = std::pair<const Expr *, const Expr *>;
} // namespace alpha

#endif // INTERNAL_TYPEDEFS_HPP
