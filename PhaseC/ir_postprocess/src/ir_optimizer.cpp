#include "ir_postprocess/ir_optimizer.hpp"

namespace alpha
{
IROptimizer::IROptimizer(const settings::IROpts &ir_opts) : ir_opts_(ir_opts) {}

std::vector<ir::Quad>
IROptimizer::run(std::vector<ir::Quad> quads) const
{
    while (true)
    {
        bool changed = false;
        if (ir_opts_.opt_jump_threading)
           changed |= do_jump_threading(quads);

        if (!changed)
            break;
    }
    return quads;
}

bool
IROptimizer::do_jump_threading(std::vector<ir::Quad> &quads)
{
    DMASSERT(support::is_in_numeric_range<LabelID::UnderlyingType>(quads.size()));
    bool changed_quads = false;

    VectorStack<std::size_t> jump_chain_indices;
    for (std::size_t i = 0; i < quads.size(); ++i)
    {
        if (quads[i].opcode != ir::Opcode::JUMP)
            continue;
        DMASSERT(jump_chain_indices.empty() && "Must always be emptied past while loop");

        auto quad_index_to_jump_thread = i;
        LabelID jump_target = quads[i].label;
        while (true)
        {
            const auto target_index = ir::Quad::label_to_index(jump_target);
            if (target_index >= quads.size()) // exit jump (one past last quad)
                    break;
            const auto &target_quad = quads[target_index];
            const auto is_self_jump =
                [&] { return ir::Quad::index_to_label(quad_index_to_jump_thread) == jump_target; };
            if (target_quad.opcode != ir::Opcode::JUMP || is_self_jump())
                break;
            jump_chain_indices.push(quad_index_to_jump_thread);
            quad_index_to_jump_thread = target_index;
            jump_target = target_quad.label;
        }
        changed_quads |= !jump_chain_indices.empty();
        while (!jump_chain_indices.empty())
        {
            quads[jump_chain_indices.top()].label = jump_target;
            jump_chain_indices.pop();
        }
    }
    return changed_quads;
}
} // namespace alpha
