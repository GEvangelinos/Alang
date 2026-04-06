#ifndef BYTECODE_GENERATOR_HPP
#define BYTECODE_GENERATOR_HPP

#include <vector>

#include "internal_typedefs.hpp"
#include "parser/ir_opcode_info_traits.gen.hpp"
#include "core/bytecode/vm_instructions.hpp"
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
    LabelID next_instruction_label_ = 0;

    BytecodeGenerator() = default;

    [[nodiscard]] vm::Program build_program(const std::vector<ir::Quad>& program_ir_quads);

    [[nodiscard]] const vm::Argument* make_operand(const Expr& expr);
    [[nodiscard]] vm::Program::StringID intern_string_literal(const ConstStringExpr& string_expr);
    [[nodiscard]] vm::Program::LibfuncID intern_libfunc_name(const LibFuncExpr& libfunc_expr);
    [[nodiscard]] LabelID next_instruction_label() noexcept { return ++next_instruction_label_; }

    template <ir::Opcode ir_quad_opcode, ir::info_traits::Requirement (*trait_func)(ir::Opcode)>
    [[nodiscard]] const vm::Argument* extract_operant_by_trait(const Expr* e);

    template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
    void generate(const ir::Quad& ir_quad);
    template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
    void generate_relational(const ir::Quad& quad);
    void generate_getretval(const ir::Quad& quad);
};
} // namespace alpha

#endif // BYTECODE_GENERATOR_HPP
