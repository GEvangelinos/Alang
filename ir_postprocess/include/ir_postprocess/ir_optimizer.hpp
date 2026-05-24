#ifndef IR_OPTIMIZER_HPP
#define IR_OPTIMIZER_HPP

#include <vector>
#include "settings/compiler_settings.hpp"
#include "core/ir/ir_quad.hpp"

namespace alpha
{
class IROptimizer
{
public:
    explicit IROptimizer(const settings::IROpts &ir_opts);

    [[nodiscard]] ir::QuadStream run(ir::QuadStream quads) const;

private:
    const settings::IROpts ir_opts_;

    [[nodiscard]] static ir::QuadStream hoist_functions(const ir::QuadStream& unhoisted_stream);
    [[nodiscard]] static bool do_dead_func_elimination(ir::QuadStream &qstream);
    [[nodiscard]] static bool do_dead_code_elimination(ir::QuadStream &qstream);
    [[nodiscard]] static bool do_jump_threading(ir::QuadStream &quads);
    [[nodiscard]] static bool do_neighbor_jump_removal(ir::QuadStream &qstream);
};
} // namespace alpha
#endif // IR_OPTIMIZER_HPP
