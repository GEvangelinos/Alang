#ifndef ID_HPP
#define ID_HPP

#include "core/numeric_types.hpp"

namespace alpha::vm
{
    enum class LibFuncId : u32
    {
        #define X(id, name) id,
        #include "libfuncs.def"
        #undef  X
        COUNT_,
    };

} // namespace alpha::vm
#endif // ID_HPP
