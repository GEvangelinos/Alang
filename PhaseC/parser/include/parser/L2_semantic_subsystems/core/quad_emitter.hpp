#ifndef QUAD_EMITTER_HPP
#define QUAD_EMITTER_HPP

#include <vector>

#include "internal_typedefs.hpp"
#include "ir_expr.hpp"
#include "core/konstants.hpp"
#include "parser/ir_opcode.gen.hpp"
#include "parser/konstants.hpp"

namespace alpha
{
class ParseCtx;

class QuadEmitter : private Immobile
{
public:
    class EmitterKey // Passkey mechanisms, so only friends to EmitterKey can emit
    {
        friend class QuadYielder;
        friend class ExprNormalizer;
        EmitterKey() = default;
    };

    QuadEmitter() = default;
    ~QuadEmitter() = default;

    void emit(
        ir::Opcode opc,
        const Expr *result,
        const Expr *arg1,
        const Expr *arg2,
        SourceLocation loc,
        LabelID label,
        EmitterKey);

    void labelPatch_quad(LabelID target_quad_label, LabelID destination_label);
    void labelPatch_list(const std::vector<LabelID> &patch_list, LabelID destination_label);
    void locPatch_tablecreate(LabelID target_quad_label, SourceLocation new_loc);

    [[nodiscard]] LabelID next_quad_label() const noexcept { return next_quad_label_; }
    [[nodiscard]] const std::vector<Quad> &quads() const noexcept { return quads_; }

private:
    LabelID next_quad_label_ = k_first_label;
    // TODO: write a container like deque but more efficient. (this can only store 9 or 10 elems before new malloc())
    std::vector<Quad> quads_;

    [[nodiscard]] static std::size_t label_to_index(LabelID label);
};

inline std::size_t
QuadEmitter::label_to_index(const LabelID label)
{
    DEBUG_SMART_ASSERT(label != k_no_label);
    return label - 1;
}
} // namespace alpha
#endif // QUAD_EMITTER_HPP
