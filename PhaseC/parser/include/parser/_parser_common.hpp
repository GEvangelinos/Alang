#ifndef _PARSER_COMMON_HPP
#define _PARSER_COMMON_HPP

#include <string>
#include "core/source_location.hpp"

namespace Alpha
{
        class Parameter
        {
        public:
                const std::string name;
                const SourceLocation location;

                Parameter(const std::string &name, SourceLocation location) : name(name), location(location) {}
        };

        struct BlockLocation
        {
                SourceLocation begin;
                SourceLocation end;
        };
} // namespace Alpha
#endif //_PARSER_COMMON_HPP
