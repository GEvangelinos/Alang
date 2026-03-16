#include "L2_semantic_subsystems/core/quad_handler.hpp"

#include "core/ir/ir_expr.hpp"
#include "core/ir/ir_quad.hpp"
#include "parser/ir_opcode_info_traits.gen.hpp"

namespace alpha
{
void
QuadHandler::emit(
    const ir::Opcode opc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc,
    const LabelID label,
    const bool is_dead,
    QuadHandler::EmitKey)
{
    #ifdef DEBUG_MODE
    namespace ii = ir::info_traits;
    using Requirement = ii::Requirement;

    auto requirement_matches = [](const Requirement req, const Expr *const expr) -> bool
    {
        return req == Requirement::OPTIONAL ||
               (req == Requirement::NONE && !expr) ||
               (req == Requirement::REQUIRED && !!expr);
    };

    DEBUG_SMART_ASSERT(
        ir_quads_.size() + 1 == next_quad_label_,
        !ir::info_traits::is_non_emittable(opc),
        requirement_matches(ii::result(opc), result),
        requirement_matches(ii::arg1(opc), arg1),
        requirement_matches(ii::arg2(opc), arg2),
    );
    if (opc != ir::Opcode::TABLECREATE)
        DEBUG_SMART_ASSERT(
        loc != SourceLocation::none()
    );
    if (opc == ir::Opcode::JUMP && label == next_quad_label())
        DEBUG_SMART_ASSERT(
        false && "ir::Opcode::JUMP jumps to itself"
    );
    #endif

    ir_quads_.emplace_back(ir::Quad{
        .loc = loc,
        .result = result,
        .arg1 = arg1,
        .arg2 = arg2,
        .label = label,
        .opcode = opc,
        .is_dead = is_dead
    });
    ++next_quad_label_;
}

void
QuadHandler::labelPatch_quad(const LabelID target_quad_label, const LabelID destination_label)
{
    // First quad at index 0, has quad with label 1.
    const u32 idx = ir::Quad::label_to_index(target_quad_label);
    DEBUG_SMART_ASSERT(
        idx < ir_quads_.size(),
        ir_quads_[idx].label == k_no_label,
        destination_label != k_no_label
    );
    ir_quads_[idx].label = destination_label;
}

void
QuadHandler::labelPatch_list(
    const std::vector<LabelID> &patch_list,
    const LabelID destination_label)
{
    DEBUG_SMART_ASSERT(destination_label != k_no_label);
    for (const LabelID target_quad_label : patch_list)
        labelPatch_quad(target_quad_label, destination_label);
}

void
QuadHandler::locPatch_tablecreate(const LabelID target_quad_label, const SourceLocation new_loc)
{
    DEBUG_SMART_ASSERT(
        target_quad_label != k_no_label && "Can't loc-patch quad without valid LabelID",
        new_loc != SourceLocation::none() && "Can't loc-patch quad without valid SourceLocation"
    );

    const u32 idx = ir::Quad::label_to_index(target_quad_label);

    // Keep asserts separate, as dereferencing might segfault
    DEBUG_SMART_ASSERT(idx < ir_quads_.size());
    DEBUG_SMART_ASSERT(
        ir_quads_[idx].opcode == ir::Opcode::TABLECREATE && "Only loc-patching tablecreate quads",
        ir_quads_[idx].loc == SourceLocation::none() &&
        "SourceLocation is already assigned, should'nt be called"
    );

    ir_quads_[idx].loc = new_loc;
}

std::vector<ir::Quad>
QuadHandler::extract_quads() noexcept
{
    const auto result = std::move(ir_quads_);
    ir_quads_.clear();
    return result;
}
} // namespace alpha
