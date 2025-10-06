#include "L2_semantic_subsystems/core/quad_emitter.hpp"

#include "parser/ir_opcode_info_traits.gen.hpp"

namespace alpha
{
void
QuadEmitter::emit(
    const ir::Opcode opc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc,
    const LabelID label,
    QuadEmitter::EmitterKey)
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

    SMART_ASSERT(
        quads_.size() + 1 == next_quad_label_,
        !ir::info_traits::is_non_emittable(opc),
        requirement_matches(ii::result(opc), result),
        requirement_matches(ii::arg1(opc), arg1),
        requirement_matches(ii::arg2(opc), arg2),
    );
    if (opc != ir::Opcode::TABLECREATE)
        SMART_ASSERT(
        loc != k_no_loc
    );
    if (opc == ir::Opcode::JUMP && label == next_quad_label())
        SMART_ASSERT(
        false && "ir::Opcode::JUMP jumps to itself"
    );
    #endif

    quads_.emplace_back(Quad{
        .loc = loc,
        .label = label,
        .result = result,
        .arg1 = arg1,
        .arg2 = arg2,
        .opcode = opc,

    });
    ++next_quad_label_;
}

void
QuadEmitter::labelPatch_quad(const LabelID target_quad_label, const LabelID destination_label)
{
    // First quad at index 0, has quad with label 1.
    const u32 idx = QuadEmitter::label_to_index(target_quad_label);
    DEBUG_SMART_ASSERT(
        idx < quads_.size(),
        quads_[idx].label == k_no_label,
        destination_label != k_no_label
    );
    quads_[idx].label = destination_label;
}

void
QuadEmitter::labelPatch_list(
    const std::vector<LabelID> &patch_list,
    const LabelID destination_label)
{
    DEBUG_SMART_ASSERT(destination_label != k_no_label);
    for (const LabelID target_quad_label: patch_list)
        labelPatch_quad(target_quad_label, destination_label);
}

void
QuadEmitter::locPatch_tablecreate(const LabelID target_quad_label, const SourceLocation new_loc)
{
    DEBUG_SMART_ASSERT(
        target_quad_label != k_no_label && "Can't loc-patch quad without valid LabelID",
        new_loc != k_no_loc && "Can't loc-patch quad without valid SourceLocation"
    );

    const u32 idx = QuadEmitter::label_to_index(target_quad_label);

    // Keep asserts separate, as dereferencing might segfault
    DEBUG_SMART_ASSERT(idx < quads_.size());
    DEBUG_SMART_ASSERT(
        quads_[idx].opcode == ir::Opcode::TABLECREATE && "Only loc-patching tablecreate quads",
        quads_[idx].loc == k_no_loc && "SourceLocation is already assigned, should'nt be called"
    );

    quads_[idx].loc = new_loc;
}
} // namespace alpha
