#ifndef QUAD_HANDLER_HPP
#define QUAD_HANDLER_HPP

#include <vector>
#include "parser/ir.hpp"
#include "parser/konstants.hpp"

namespace Alpha
{
class QuadHandler : private Immobile
{
public:
    QuadHandler() = default;
    ~QuadHandler() = default;

    void emit_next(
        IOPCode iopc,
        const Expr *result,
        const Expr *arg1,
        const Expr *arg2,
        SourceLocation loc,
        LabelID label_offset = 0);

    void emit_labelless(
        IOPCode iopc,
        const Expr *result,
        const Expr *arg1,
        const Expr *arg2,
        SourceLocation loc);

    void patch_quad(LabelID target_quad_label, LabelID destination_label);
    void patch_list(const std::vector<LabelID> &patch_list, LabelID destination_label);

    [[nodiscard]] LabelID next_quad_label() const noexcept { return next_quad_label_; }
    [[nodiscard]] const std::vector<Quad> &quads() const noexcept { return quads_; }

private:
    LabelID next_quad_label_ = k_first_label;
    std::vector<Quad> quads_;

    void emit(
        IOPCode iopc,
        const Expr *result,
        const Expr *arg1,
        const Expr *arg2,
        LabelID label,
        SourceLocation loc);

};

inline void
QuadHandler::emit(
    const IOPCode iopc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const LabelID label,
    const SourceLocation loc)
{
    DEBUG_SMART_ASSERT(quads_.size() + 1 == next_quad_label_);
    quads_.emplace_back(Quad{
        .iopcode = iopc,
        .arg1 = arg1,
        .arg2 = arg2,
        .result = result,
        .label = label,
        .location = loc,
    });
    ++next_quad_label_;
}

inline void
QuadHandler::emit_next(
    const IOPCode iopc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc,
    const LabelID label_offset)
{
    emit(iopc, result, arg1, arg2, next_quad_label_ + label_offset, loc);
}

inline void
QuadHandler::emit_labelless(
    const IOPCode iopc,
    const Expr *const result,
    const Expr *const arg1,
    const Expr *const arg2,
    const SourceLocation loc)
{
    emit(iopc, result, arg1, arg2, k_no_label, loc);
}

inline void
QuadHandler::patch_quad(const LabelID target_quad_label, const LabelID destination_label)
{
    // First quad at index 0, has quad with label 1.
    const u32 quad_index = target_quad_label - 1;
    DEBUG_SMART_ASSERT(target_quad_label > 0,
                       quad_index < quads_.size(),
                       quads_[quad_index].label == k_no_label,
                       destination_label != k_no_label
    );
    quads_[quad_index].label = destination_label;
}

inline void
QuadHandler::patch_list(const std::vector<LabelID> &patch_list, const LabelID destination_label)
{
    DEBUG_SMART_ASSERT(destination_label != k_no_label);
    for (const LabelID target_quad_label: patch_list)
        patch_quad(target_quad_label, destination_label);
}
} // namespace Alpha
#endif // QUAD_HANDLER_HPP
