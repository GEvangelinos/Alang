#ifndef _PARSER_COMMON_HPP
#define _PARSER_COMMON_HPP
#include <string>
#include "core/alpha_location.hpp"

namespace Alpha
{
        class Parameter
        {
        private:
                const std::string name_;
                const Location location_;

        public:
                Parameter(const std::string &name, Location location)
                    : name_(name), location_(location) {}

                DEBUG_ALWAYS_INLINE const std::string &name() const noexcept { return name_; }
                DEBUG_ALWAYS_INLINE Location location() const noexcept { return location_; }
        };
} // namespace Alpha
#endif //_PARSER_COMMON_HPP
