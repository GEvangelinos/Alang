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

    [[nodiscard]] std::vector<ir::Quad> run(std::vector<ir::Quad> quads) const;

private:
    const settings::IROpts ir_opts_;

    static bool do_jump_threading(std::vector<ir::Quad> &quads);
    static bool do_unused_temp_removal(std::vector<ir::Quad> &quads);
};
} // namespace alpha
#endif // IR_OPTIMIZER_HPP
