#ifndef _PARSER_COMMON_HPP
#define _PARSER_COMMON_HPP

#include <string>
#include <vector>
#include "core/source_location.hpp"


namespace alpha
{
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
