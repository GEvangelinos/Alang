#include "ir_postprocess/ir_optimizer.hpp"
#include "parser/ir_opcode_info_traits.gen.hpp"
#include "ir_editor.hpp"

namespace alpha
{
bool
IROptimizer::do_neighbor_jump_removal(ir::QuadStream& qstream)
{
    struct SourcedQuad
    {
        const ir::Quad* ptr;
        u64 quad_idx;
    };

    if (qstream.empty()) return false;

    IREditor ir_editor(qstream);

    SourcedQuad latest_useful_quad{
        .ptr = &qstream.back(),
        .quad_idx = qstream.size()
    };

    for (i64 i = qstream.size() - 1; i >= 0; --i)
    {
        ir::Quad& q = qstream[i];

        if (ir::info_traits::is_branching(q.opcode) &&
            q.label == ir::Quad::index_to_label(latest_useful_quad.quad_idx)
        ) { ir_editor.kill(i); }
        else
            latest_useful_quad = SourcedQuad{.ptr = &q, .quad_idx = static_cast<u64>(i)};
    }

    return ir_editor.apply();
}
} // namespace alpha
