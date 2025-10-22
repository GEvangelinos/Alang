#ifndef IR_OPTIMIZER_HPP
#define IR_OPTIMIZER_HPP

#include <vector>
#include "settings/compiler_settings.hpp"

#include "ir_quad.hpp"

namespace alpha
{
class IROptimizer
{
public:
    explicit IROptimizer(const settings::IROpts &ir_opts);

    [[nodiscard]] std::vector<Quad> run(std::vector<Quad> quads) const;

private:
    const settings::IROpts ir_opts_;

    static bool do_jump_threading(std::vector<Quad> &quads);
    static bool do_unused_temp_removal(std::vector<Quad> &quads);
};
} // namespace alpha
#endif // IR_OPTIMIZER_HPP
