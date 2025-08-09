#ifndef _PARSER_COMMON_HPP
#define _PARSER_COMMON_HPP

#include <string>
#include "core/source_location.hpp"

namespace alpha
{
struct Expr;
using ExprList = std::vector<const Expr *>;
using ExprPair = std::pair<const Expr *, const Expr *>;
using DictList = std::vector<const ExprPair *>;

class Parameter
{
public:
    const std::string name;
    const SourceLocation loc;

    Parameter(const std::string &name, const SourceLocation loc)
        : name(name), loc(loc) {}
};
} // namespace alpha
#endif //_PARSER_COMMON_HPP
