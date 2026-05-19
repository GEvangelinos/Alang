#include "core/bytecode/vm_opcodes.hpp"
#include <support/string_tools.hpp>
namespace alpha::vm
{
std::string to_string(const vm::Opcode opc) noexcept
{
    switch (opc)
    {
		#define AS_SWITCH_CASE(opc) case vm::Opcode::opc: return support::tolower_str(#opc);
        ALPHA_VMOPCODES(AS_SWITCH_CASE)
        #undef  AS_SWITCH_CASE
        default:
        UNREACHABLE(FMT::format(
            "BUG: Unknown Opcode. Opcode's int value = `{}`", static_cast<int>(opc)));
    }
}
} // namespace alpha::vm

