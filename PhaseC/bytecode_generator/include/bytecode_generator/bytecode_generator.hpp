#ifndef BYTECODE_GENERATOR_HPP
#define BYTECODE_GENERATOR_HPP

#include <vector>

#include "internal_typedefs.hpp"
#include "parser/ir_opcode_info_traits.gen.hpp"
#include "core/bytecode/vm_instruction.hpp"
#include "core/bytecode/vm_program.hpp"
#include "core/ir/ir_expr.hpp"
#include "core/ir/ir_quad.hpp"

namespace alpha
{
class BytecodeGenerator
{
public:
    [[nodiscard]] static vm::Program run(const std::vector<ir::Quad>& program_ir_quads)
    {
        return BytecodeGenerator{}.build_program(program_ir_quads);
    }

private:
    vm::Program result_;
    std::vector<LabelID> target_addresses_;

    BytecodeGenerator() = default;

    [[nodiscard]] vm::Program build_program(const std::vector<ir::Quad>& program_ir_quads) &&;

    [[nodiscard]] const vm::Argument* make_argument(const Expr& expr);
    [[nodiscard]] vm::Program::StringID intern_string_literal(const ConstStringExpr& string_expr);
    [[nodiscard]] vm::Program::LibfuncID intern_libfunc_name(const LibFuncExpr& libfunc_expr);

    [[nodiscard]] LabelID next_instruction_label() const noexcept
    {
        DMASSERT(result_.code.size() <= std::numeric_limits<LabelID::UnderlyingType>::max());
        return LabelID{static_cast<LabelID::UnderlyingType>(result_.code.size())};
    }

    template <ir::Opcode ir_quad_opcode, ir::info_traits::Requirement (*trait_func)(ir::Opcode)>
    [[nodiscard]] const vm::Argument* extract_operant_by_requirement_trait(const Expr* e);

    template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
    void generate(const ir::Quad& q);
    template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
    void generate_relational(const ir::Quad& q);
    void generate_getretval(const ir::Quad& q);
    void generate_funcstart(const ir::Quad& quad);
    void generate_funcend(const ir::Quad& quad);
    void generate_return(const ir::Quad& quad);
};
} // namespace alpha

#endif // BYTECODE_GENERATOR_HPP
