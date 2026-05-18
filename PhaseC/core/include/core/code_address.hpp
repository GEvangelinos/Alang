#ifndef CODE_ADDRESS_HPP
#define CODE_ADDRESS_HPP

#include "numeric_types.hpp"
#include "strong_type.hpp"

namespace alpha
{
struct CodeAddress : StrongType<CodeAddress, u32>
{
    using StrongType::StrongType;
};
} // namespace alpha
#endif // CODE_ADDRESS_HPP