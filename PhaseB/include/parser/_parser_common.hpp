#ifndef _PARSER_COMMON_HPP
#define _PARSER_COMMON_HPP
#include <string>
#include "core/alpha_location.hpp"

namespace Alpha
{
        class Parameter
        {
        public:
                const std::string &name_;
                const CodeLocation location_;

                Parameter(const std::string &name, CodeLocation location)
                    : name_(name), location_(location) {}
        };
} // namespace Alpha
#endif //_PARSER_COMMON_HPP
