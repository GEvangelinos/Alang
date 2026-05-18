#ifndef ABC_GENERATOR_HPP
#define ABC_GENERATOR_HPP

#include <vector>

#include "internal_typedefs.hpp"
#include "parser/ir_opcode_info_traits.gen.hpp"
#include "core/bytecode/vm_instruction.hpp"
#include "core/bytecode/vm_program.hpp"
#include "core/ir/ir_expr.hpp"
#include "core/ir/ir_quad.hpp"

namespace alpha
{
class ABC_Generator
{
public:
    [[nodiscard]] static vm::Program run(const ir::QuadStream& program_ir_quads)
    {
        return ABC_Generator{}.build_program(program_ir_quads);
    }

private:
    vm::Program result_;
    std::vector<CodeAddress> target_addresses_;

    ABC_Generator() = default;

    [[nodiscard]] vm::Program build_program(const ir::QuadStream& program_ir_quads) &&;

    [[nodiscard]] const vm::Argument* make_argument(const Expr& expr);
    [[nodiscard]] vm::Program::StringID intern_string_literal(const ConstStringExpr& string_expr);
    [[nodiscard]] vm::Program::LibfuncID intern_libfunc_name(const LibFuncExpr& libfunc_expr);

    [[nodiscard]] CodeAddress next_instruction_label() const noexcept
    {
        DMASSERT(result_.instructions.size() <= std::numeric_limits<CodeAddress::UnderlyingType>::max());
        return CodeAddress{static_cast<CodeAddress::UnderlyingType>(result_.instructions.size())};
    }

    template <ir::Opcode ir_quad_opcode, ir::info_traits::Requirement (*trait_func)(ir::Opcode)>
    [[nodiscard]] const vm::Argument* extract_operant_by_requirement_trait(const Expr* e);

    template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
    void generate(const ir::Quad& q);
    template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
    void generate_relational(const ir::Quad& q);
    void generate_uminus(const ir::Quad& q);
    void generate_getretval(const ir::Quad& q);
    void generate_funcstart(const ir::Quad& quad);
    void generate_return(const ir::Quad& quad);
};
} // namespace alpha

#endif // ABC_GENERATOR_HPP
