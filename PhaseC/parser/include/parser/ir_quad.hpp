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

namespace alpha
{
struct Quad // Physical layout (packed): 8B first, then 4B, then 1B
{
    SourceLocation loc;
    const Expr *result;
    const Expr *arg1;
    const Expr *arg2;
    LabelID label = k_no_label;
    const ir::Opcode opcode;
    const bool is_dead;

    static std::size_t label_to_index(LabelID label);
    static LabelID index_to_label(std::size_t index);
};

inline std::size_t
Quad::label_to_index(const LabelID label)
{
    DEBUG_SMART_ASSERT(label != k_no_label);
    return label - 1;
}

inline LabelID
Quad::index_to_label(const std::size_t index) { return index + 1; }
} // namespace alpha
#endif // IR_QUAD_HPP
