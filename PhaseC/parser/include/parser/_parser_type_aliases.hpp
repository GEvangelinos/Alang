#ifndef _PARSER_TYPE_ALIASES_HPP
#define _PARSER_TYPE_ALIASES_HPP

#include <vector>

namespace Alpha
{
        struct Expr;
        using ExprList = std::vector<Expr *>;
        using ExprPair = std::pair<Expr *, Expr *>;
        using DictList = std::vector<ExprPair *>;
}
#endif // _PARSER_TYPE_ALIASES_HPP
