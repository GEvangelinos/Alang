#ifndef ALPHA_DEFS_HPP
#define ALPHA_DEFS_HPP

#include <cstdint>

namespace Alpha
{
        typedef struct
        {
                std::uint32_t line;
                std::uint32_t column;
                std::uint32_t index;
        } InputBufferContext;

        /* Indicies are used to quickly track the location
         * of the parsed rule  inside the input buffer (char *).
         */
        typedef struct
        {
                uint32_t first_line;
                uint32_t first_column;
                uint32_t first_index;
                uint32_t last_line;
                uint32_t last_column;
                uint32_t last_index;
        } Location;
}

#endif /* ALPHA_DEFS_HPP */
