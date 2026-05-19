#ifndef _PARSER_COMMON_HPP
#define _PARSER_COMMON_HPP

#include <string>
#include <vector>
#include "core/source_location.hpp"
#include "core/string_span.hpp"


namespace alpha
{
class Parameter
{
public:
    const StringSpan name;
    const SourceLocation loc;

    Parameter(const StringSpan name, const SourceLocation loc)
        : name(name), loc(loc) {}
};
} // namespace alpha
#endif //_PARSER_COMMON_HPP
