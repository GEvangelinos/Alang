#include "ir_postprocess/ir_optimizer.hpp"

namespace alpha
{
IROptimizer::IROptimizer(const settings::IROpts& ir_opts)
    : ir_opts_(ir_opts) {}

ir::QuadStream
IROptimizer::run(ir::QuadStream quads) const
{
    if (ir_opts_.opt_func_hoisting)
        quads = hoist_functions(quads);

    while (true)
    {
        bool changed = false;
        if (ir_opts_.opt_jump_threading)
            changed |= do_jump_threading(quads);
        if (ir_opts_.opt_neighbor_jumps)
            changed |= do_neighbor_jump_removal(quads);
        if (ir_opts_.opt_dead_func_elimination)
            changed |= do_dead_func_elimination(quads);
        if (!changed)
            break;
    }
    return quads;
}
} // namespace alpha
