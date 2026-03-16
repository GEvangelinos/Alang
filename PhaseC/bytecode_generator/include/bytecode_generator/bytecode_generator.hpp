#ifndef BYTECODE_GENERATOR_HPP
#define BYTECODE_GENERATOR_HPP

#include <vector>

#include "internal_typedefs.hpp"
#include "core/string_span.hpp"
#include "core/bytecode/vm_instructions.hpp"
#include "core/ir/ir_expr.hpp"
#include "core/ir/ir_quad.hpp"

namespace alpha
{
class BytecodeGenerator
{
public:




private:
    struct UserFunc
    {
        u32 address;
        u32 local_size;
        StringSpan id;
    };

    // TODO: is it possible to inline the numbers? (Integers and/or floats?)
    std::vector<StringSpan> string_literal_pool_;
    std::vector<StringSpan> libfunc_name_pool_;
    std::vector<UserFunc> userfunc_table;

    std::vector<vm::Instruction> vm_instructions_;

    [[nodiscard]] const vm::Argument* make_operand(const Expr& expr);
    [[nodiscard]] u32 intern_string_literal(const ConstStringExpr & string_expr);
    [[nodiscard]] u32 intern_libfunc_name(const LibFuncExpr & libfunc_expr);

    void generate(vm::Opcode opcode, const ir::Quad& ir_quad);
};
} // namespace alpha

#endif // BYTECODE_GENERATOR_HPP
