#ifndef ABC_SERIALIZER_HPP
#define ABC_SERIALIZER_HPP

#include "core/numeric_types.hpp"
#include "core/bytecode/vm_program.hpp"

namespace alpha
{
class ABC_Serializer
{
public:
    [[nodiscard]] static std::vector<u8> serialize(const vm::Program& program);

private:
    ABC_Serializer() = default;
};
} // namespace alpha
#endif // ABC_SERIALIZER_HPP
