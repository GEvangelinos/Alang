#ifndef ALPHA_DEFS_HPP
#define ALPHA_DEFS_HPP

#include <cstdint>
#include <string>

namespace Alpha
{
        struct InputBufferContext
        {
                std::uint32_t line;
                std::uint32_t column;
                std::uint32_t index;
                const std::string filename;

                InputBufferContext() = delete;
                InputBufferContext(std::string filename)
                    : line(1),
                      column(1),
                      index(0),
                      filename(filename) {}
        };

        /* Indicies are used to quickly track the location
         * of the parsed rule  inside the input buffer (char *).
         */
        struct Location
        {
                std::uint32_t first_line;
                std::uint32_t first_column;
                std::uint32_t first_index;
                std::uint32_t last_line;
                std::uint32_t last_column;
                std::uint32_t last_index;
        };
}

#endif /* ALPHA_DEFS_HPP */
