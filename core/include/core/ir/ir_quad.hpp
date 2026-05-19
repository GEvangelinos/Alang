#ifndef IR_QUAD_HPP
#define IR_QUAD_HPP
#include <optional>
#include <vector>
#include <parser/konstants.hpp>
#include "core/basics.hpp"
#include "core/numeric_types.hpp"
#include "parser/symbols.hpp"

#include "core/konstants.hpp"
#include "parser/ir_opcode.gen.hpp"
#include "support/misc_tools.hpp"
#include "support/string_tools.hpp"

namespace alpha::ir
{
struct Quad // Physical layout (packed): 8B first, then 4B, then 1B
{
    SourceLocation loc;
    const Expr* result;
    const Expr* arg1;
    const Expr* arg2;
    CodeAddress label = CodeAddress::none();
    const ir::Opcode opcode;
    const bool is_dead;

    static std::size_t label_to_index(CodeAddress label);
    static CodeAddress index_to_label(std::size_t index);
};

inline std::size_t
Quad::label_to_index(const CodeAddress label)
{
    DMASSERT(label != CodeAddress::none());
    return label.value - 1;
}

inline CodeAddress
Quad::index_to_label(const std::size_t index)
{
    static_assert(std::is_integral_v<CodeAddress::UnderlyingType>);
    // If fals following assertion fails
    DMASSERT(index < std::numeric_limits<CodeAddress::UnderlyingType>::max());
    return CodeAddress{static_cast<CodeAddress::UnderlyingType>(index + 1)};
}

using QuadStream = std::vector<Quad>;
} // namespace alpha::ir
#endif // IR_QUAD_HPP
