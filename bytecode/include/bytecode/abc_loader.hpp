#ifndef ABC_LOADER_HPP
#define ABC_LOADER_HPP

#include <vector>

#include "executable.hpp"
#include "core/numeric_types.hpp"

namespace alpha
{
class ABC_Loader
{
public:
    [[nodiscard]] static vm::Executable load(const std::vector<u8> &byte_buffer);

private:
    ABC_Loader() = default;
};
} // namespace alpha

#endif // ABC_LOADER_HPP
