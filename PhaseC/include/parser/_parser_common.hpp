#ifndef _PARSER_COMMON_HPP
#define _PARSER_COMMON_HPP

#include "core/alpha_location.hpp"
#include <string>
#include <vector>

namespace Alpha
{
        // Classes defined here:
        class Parameter;

        class Parameter
        {
        public:
                const std::string name;
                const Location location;

                Parameter(const std::string &name, Location location) : name(name), location(location) {}
        };

        struct BlockLocation
        {
                Location begin;
                Location end;
        };

        [[nodiscard]] bool is_modifiable_symbol(const Symbol *const symbol)
        {
                // TODO: remove (deprecated part from phase 2)
                // if (!symbol) // nullptr implies runtime-evaluated lvalue (e.g. member access)
                // 	return true;
                return symbol->is_variable();
        }

} // namespace Alpha
#endif //_PARSER_COMMON_HPP
