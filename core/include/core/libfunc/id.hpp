#ifndef ID_HPP
#define ID_HPP

#include "core/numeric_types.hpp"
#include "support/format_adapter.hpp"

namespace alpha::vm
{
    enum class LibFuncId : u32
    {
        #define X(id, name) id,
        #include "libfuncs.def"
        #undef  X
        __COUNT__,
    };

    using LibFuncIdUT = std::underlying_type_t<LibFuncId>;

} // namespace alpha::vm

// Formatter for LibFuncID so FMT::format() knows how to format LibFuncId
template <>
struct FMT::formatter<alpha::vm::LibFuncId> : FMT::formatter<alpha::vm::LibFuncIdUT>
{
    // Don't make static if clang-tidy prompts you (it's against the standard) (It must be a const method)
    auto format(const alpha::vm::LibFuncId id, FMT::format_context& ctx) const
    {
        return FMT::formatter<alpha::vm::LibFuncIdUT>::format(static_cast<alpha::vm::LibFuncIdUT>(id), ctx);
    }
};

#endif // ID_HPP
