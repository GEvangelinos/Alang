#ifndef QUAD_HANDLER_HPP
#define QUAD_HANDLER_HPP

#include <vector>

#include "internal_typedefs.hpp"
#include "ir_quad.hpp"
#include "parser/ir_opcode.gen.hpp"
#include "parser/konstants.hpp"

namespace alpha
{
class ParseCtx;

class QuadHandler : private Immobile
{
public:
    class EmitKey // Passkey mechanisms, so only friends to EmitKey can emit
    {
        friend class QuadInterceptor;
        EmitKey() = default;
    };

    QuadHandler() = default;
    ~QuadHandler() = default;

    void emit(
        ir::Opcode opc,
        const Expr *result,
        const Expr *arg1,
        const Expr *arg2,
        SourceLocation loc,
        LabelID label,
        EmitKey);

    void labelPatch_quad(LabelID target_quad_label, LabelID destination_label);
    void labelPatch_list(const std::vector<LabelID> &patch_list, LabelID destination_label);
    void locPatch_tablecreate(LabelID target_quad_label, SourceLocation new_loc);

    [[nodiscard]] LabelID next_quad_label() const noexcept { return next_quad_label_; }
    [[nodiscard]] std::vector<Quad> extract_quads() noexcept;

private:
    LabelID next_quad_label_ = k_first_label;
    // TODO: write a container like deque but more efficient. (this can only store 9 or 10 elems before new malloc())
    std::vector<Quad> quads_;
};

} // namespace alpha
#endif // QUAD_HANDLER_HPP
