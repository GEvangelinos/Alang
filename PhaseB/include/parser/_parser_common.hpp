#ifndef _PARSER_COMMON_HPP
#define _PARSER_COMMON_HPP
#include <string>
#include "core/alpha_location.hpp"

namespace Alpha
{
        // Classes defined here:
        class Parameter;

        class Parameter
        {
        public:
                const std::string name;
                const Location location;

                Parameter(const std::string &name, Location location)
                    : name(name), location(location) {}
        };
} // namespace Alpha
#endif //_PARSER_COMMON_HPP
