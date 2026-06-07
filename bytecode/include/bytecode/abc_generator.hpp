#ifndef ABC_GENERATOR_HPP
#define ABC_GENERATOR_HPP

#include <vector>

#include "internal_typedefs.hpp"
#include "parser/ir_opcode_info_traits.gen.hpp"
#include "core/bytecode/vm_instruction.hpp"
#include "core/bytecode/vm_program.hpp"
#include "core/ir/ir_expr.hpp"
#include "core/ir/ir_quad.hpp"
#include "core/libfunc/mappings.hpp"

namespace alpha
{
class ABC_Generator
{
public:

    struct Config
    {
        const ir::QuadStream& qstream;
        const u32 global_var_count;
    };

    [[nodiscard]] static vm::Program run(const ABC_Generator::Config &config)
    {
        return ABC_Generator{config}.build_program();
    }

private:
    const Config config_;
    vm::Program result_;
    std::vector<CodeAddress> target_addresses_;

    explicit ABC_Generator(const Config &config) ;

    [[nodiscard]] vm::Program build_program() &&;

    [[nodiscard]] std::unique_ptr<vm::Argument> make_argument(const Expr& expr);
    [[nodiscard]] vm::StringID intern_string_literal(const ConstStringExpr& string_expr);
    [[nodiscard]] vm::StringID intern_progfunc_name(const ProgFuncExpr& progfunc_expr);
    [[nodiscard]] vm::LibFuncId retrieve_libfunc_id(const LibFuncExpr& libfunc_expr);

    [[nodiscard]] CodeAddress next_instruction_label() const noexcept
    {
        DMASSERT(result_.instructions.size() <= std::numeric_limits<CodeAddress::UnderlyingType>::max());
        return CodeAddress{static_cast<CodeAddress::UnderlyingType>(result_.instructions.size())};
    }

    template <ir::Opcode ir_quad_opcode, ir::info_traits::Requirement (*trait_func)(ir::Opcode)>
    [[nodiscard]] std::unique_ptr<vm::Argument> extract_operant_by_requirement_trait(const Expr* e);

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
