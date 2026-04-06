#include "bytecode_generator/bytecode_generator.hpp"

#include <assert.h>

#include "parser/ir_opcode_info_traits.gen.hpp"
#include "core/ir/ir_expr.hpp"
#include "support/dependent_false.hpp"

namespace alpha
{
vm::Program::StringID
BytecodeGenerator::intern_string_literal(const ConstStringExpr& string_expr)
{
    const auto it = result_.string_literal_table.try_emplace(
        string_expr.value, result_.string_literal_table.size()
    ).first;
    const auto str_id = it->second;
    return str_id;
}

vm::Program::LibfuncID
BytecodeGenerator::intern_libfunc_name(const LibFuncExpr& libfunc_expr)
{
    const auto it = result_.libfunc_name_table.try_emplace(
        libfunc_expr.libfunc_symbol->name, result_.libfunc_name_table.size()
    ).first;
    const auto libname_id = it->second;
    return libname_id;
}


const vm::Argument*
BytecodeGenerator::make_operand(const Expr& expr)
{
    using ET = Expr::Type;
    switch (expr.type)
    {
    case ET::CONST_BOOL:
        return new vm::ConstBoolArgument{static_cast<const ConstBoolExpr&>(expr).value};
    case ET::CONST_INT:
        return new vm::ConstIntArgument{static_cast<const ConstIntExpr&>(expr).value};
    case ET::CONST_FLOAT:
        return new vm::ConstFloatArgument{static_cast<const ConstFloatExpr&>(expr).value};
    case ET::CONST_STRING:
        return new vm::ConstStringArgument{
            intern_string_literal(static_cast<const ConstStringExpr&>(expr))
        };
    case ET::CONST_NIL:
        return new vm::ConstNilArgument{};
    case ET::LIBRARY_FUNCTION:
        return new vm::LibFuncArgument{intern_libfunc_name(static_cast<const LibFuncExpr&>(expr))};
    case ET::PROGRAM_FUNCTION:
        return new vm::ProgramFuncArgument{
            static_cast<const ProgFuncExpr&>(expr).progfunc_symbol->address
        };
    case ET::ARITHMETIC:
    case ET::ASSIGN:
    case ET::BOOL:
    case ET::NEW_TABLE:
    case ET::TABLE_ITEM:
    case ET::VARIABLE:
        DEBUG_SMART_ASSERT(expr.has_var_symbol());
        switch (const auto var = static_cast<const ExprWVarSymbol&>(expr).var_symbol; var->space)
        {
        case VarSymbol::Space::PROGRAM_VAR: return new vm::GlobalVariableArgument{var->offset};
        case VarSymbol::Space::FUNCTION_LOCAL: return new vm::LocalVariableArgument{var->offset};
        case VarSymbol::Space::FORMAL_ARGUMENT: return new vm::FormalVariableArgument{var->offset};
        default: DEBUG_UNREACHABLE("Unknown VarSymbol::Space");
        }
    default:
        UNREACHABLE(FMT::format("Unknown Expr::Type: int(type) = {}", static_cast<int>(expr.type)));
    }
}

template <ir::Opcode ir_quad_opcode, ir::info_traits::Requirement (*trait_func)(ir::Opcode)>
const vm::Argument*
BytecodeGenerator::extract_operant_by_trait(const Expr* const e)
{
    namespace IIT = ir::info_traits;
    constexpr IIT::Requirement req = trait_func(ir_quad_opcode);
    if constexpr (req == IIT::Requirement::REQUIRED)
        return make_operand(*DEBUG_REQUIRE_PTR(e));
    else if constexpr (req == IIT::Requirement::NONE)
        return nullptr;
    else if constexpr (req == IIT::Requirement::OPTIONAL)
        return e ? make_operand(*e) : nullptr;
    else static_assert(always_false_v<decltype(ir_quad_opcode)>, "Unknown Requirement value");
}

template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
void
BytecodeGenerator::generate(const ir::Quad& ir_quad)
{
    DEBUG_SMART_ASSERT(ir_opcode == ir_quad.opcode);
    namespace IIT = ir::info_traits;
    result_.code.emplace_back(
        vm_opcode,
        extract_operant_by_trait<ir_opcode, IIT::result>(ir_quad.result),
        extract_operant_by_trait<ir_opcode, IIT::arg1>(ir_quad.arg1),
        extract_operant_by_trait<ir_opcode, IIT::arg2>(ir_quad.arg2),
        ir_quad.loc
    );
    target_addresses_.push_back(next_instruction_label());
}

template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
void
BytecodeGenerator::generate_relational(const ir::Quad& quad)
{
    DEBUG_SMART_ASSERT(ir_opcode == quad.opcode);
    namespace IIT = ir::info_traits;
    result_.code.emplace_back(
        vm_opcode,
        [&]()
        {
            DEBUG_SMART_ASSERT(quad.label != k_no_label && "All relational have labels");
            LabelID patch_taddress = k_no_label;
            if (quad.label < next_instruction_label_)
            {
                DEBUG_SMART_ASSERT(quad.label < target_addresses_.size());
                patch_taddress = target_addresses_[quad.label];
            }
            else
            {
                #warning "DOING NOTHING"
                // add_incomplete_jump(next_instruction_label(), );
            }
            return new vm::LabelArgument{patch_taddress};
        }(),
        extract_operant_by_trait<ir_opcode, IIT::arg1>(quad.arg1),
        extract_operant_by_trait<ir_opcode, IIT::arg2>(quad.arg2),
        quad.loc
    );
    target_addresses_.push_back(next_instruction_label());
}

void
BytecodeGenerator::generate_getretval(const ir::Quad& quad)
{
    generate<ir::Opcode::GETRETVAL, vm::Opcode::ASSIGN>(quad);
    DEBUG_SMART_ASSERT(!result_.code.empty() && !result_.code.back().arg1);
    result_.code.back().arg1 = new vm::RetvalArgument{};
}

vm::Program
BytecodeGenerator::build_program(const std::vector<ir::Quad>& program_ir_quads)
{
    for (const auto& quad : program_ir_quads)
    {
    #define CASE_BASIC(ir_op, vm_op) case ir::Opcode::ir_op: generate<ir::Opcode::ir_op, vm::Opcode::vm_op>(quad); break
    #define CASE_RELATIONAL(ir_op, vm_op) case ir::Opcode::ir_op: generate_relational<ir::Opcode::ir_op, vm::Opcode::vm_op>(quad); break
        switch (quad.opcode)
        {
        CASE_BASIC(ASSIGN, ASSIGN);
        case ir::Opcode::UMINUS: break;
        CASE_BASIC(ADD, ADD);
        CASE_BASIC(SUB, SUB);
        CASE_BASIC(MUL, MUL);
        CASE_BASIC(DIV, DIV);
        CASE_BASIC(MOD, MOD);
        CASE_RELATIONAL(IF_EQ, JEQ);
        CASE_RELATIONAL(IF_NEQ, JNE);
        CASE_RELATIONAL(IF_GT, JGT);
        CASE_RELATIONAL(IF_GTE, JGE);
        CASE_RELATIONAL(IF_LT, JLT);
        CASE_RELATIONAL(IF_LTE, JLE);
        CASE_RELATIONAL(JUMP, JUMP);
        CASE_BASIC(TABLECREATE, NEWTABLE);
        CASE_BASIC(TABLEGETELEM, TABLEGETELEM);
        CASE_BASIC(TABLESETELEM, TABLESETELEM);
        CASE_BASIC(PARAM, PUSHARG);
        CASE_BASIC(CALL, CALLFUNC);
        case ir::Opcode::GETRETVAL:
            generate_getretval(quad);
            break;
        case ir::Opcode::RETURN:
            break;
        case ir::Opcode::FUNCSTART:
            break;
        case ir::Opcode::FUNCEND:
            break;
        case ir::Opcode::NOT:
        case ir::Opcode::AND:
        case ir::Opcode::OR: DEBUG_UNREACHABLE(false && "Should be unused ir opcodes");
        }
    #undef CASE_BASIC
    #undef CASE_RELATIONAL
    }
    return std::move(result_);
}
} // namespace alpha
