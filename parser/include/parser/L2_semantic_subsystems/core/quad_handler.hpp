#ifndef QUAD_HANDLER_HPP
#define QUAD_HANDLER_HPP

#include <vector>

#include "internal_typedefs.hpp"
#include "core/ir/ir_quad.hpp"
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
        CodeAddress label,
        bool is_dead,
        EmitKey);

    void labelPatch_quad(CodeAddress target_quad_label, CodeAddress destination_label);
    void labelPatch_list(const std::vector<CodeAddress> &patch_list, CodeAddress destination_label);
    void locPatch_tablecreate(CodeAddress target_quad_label, SourceLocation new_loc);

    [[nodiscard]] CodeAddress next_quad_label() const noexcept
    {
        return CodeAddress{static_cast<CodeAddress::UnderlyingType>(ir_quads_.size() + 1)};
    }
    [[nodiscard]] ir::QuadStream extract_quads() noexcept;

private:
    ir::QuadStream ir_quads_;
};

} // namespace alpha
#endif // QUAD_HANDLER_HPP
