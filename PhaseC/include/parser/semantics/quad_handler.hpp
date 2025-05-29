#ifndef QUAD_HANDLER_HPP
#define QUAD_HANDLER_HPP

#include <vector>
#include "core/alpha_core_types.hpp"
#include "core/alpha_konstants.hpp"

namespace Alpha
{
class QuadHandler : private Immobile
{
public:
    QuadHandler() = default;
    ~QuadHandler() = default;

    void emit_quad(
        IOPCode iopc,
        const Expr *arg1,
        const Expr *arg2,
        const Expr *result,
        LabelID label,
        SourceLocation loc);

    void emit_next_quad(
        IOPCode iopc,
        const Expr *arg1,
        const Expr *arg2,
        const Expr *result,
        SourceLocation loc);

    void emit_labelless_quad(
        IOPCode iopc,
        const Expr *arg1,
        const Expr *arg2,
        const Expr *result,
        SourceLocation loc);

    LabelID next_quad_label() const noexcept { return next_quad_label_; }

private:
    LabelID next_quad_label_ = k_first_label;
    std::vector<Quad> quads_;
};

inline void QuadHandler::emit_quad(
    const IOPCode iopc,
    const Expr *arg1,
    const Expr *arg2,
    const Expr *result,
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
}

inline void QuadHandler::emit_next_quad(
    const IOPCode iopc,
    const Expr *const arg1,
    const Expr *const arg2,
    const Expr *const result,
    const SourceLocation loc)
{
    emit_quad(iopc, arg1, arg2, result, next_quad_label_, loc);
}

inline void QuadHandler::emit_labelless_quad(
    const IOPCode iopc,
    const Expr *const arg1,
    const Expr *const arg2,
    const Expr *const result,
    const SourceLocation loc)
{
    emit_quad(iopc, arg1, arg2, result, k_no_label, loc);
}
} // namespace Alpha
#endif // QUAD_HANDLER_HPP
