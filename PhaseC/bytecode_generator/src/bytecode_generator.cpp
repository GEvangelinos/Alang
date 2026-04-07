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
BytecodeGenerator::generate(const ir::Quad& q)
{
    DEBUG_SMART_ASSERT(ir_opcode == q.opcode);
    namespace IIT = ir::info_traits;
    result_.code.emplace_back(
        vm_opcode,
        extract_operant_by_trait<ir_opcode, IIT::result>(q.result),
        extract_operant_by_trait<ir_opcode, IIT::arg1>(q.arg1),
        extract_operant_by_trait<ir_opcode, IIT::arg2>(q.arg2),
        q.loc
    );
    target_addresses_.push_back(reserve_next_label());
}

template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
void
BytecodeGenerator::generate_relational(const ir::Quad& q)
{
    DEBUG_SMART_ASSERT(ir_opcode == q.opcode);
    namespace IIT = ir::info_traits;
    result_.code.emplace_back(
        vm_opcode,
        [&]()
        {
            DEBUG_SMART_ASSERT(q.label != k_no_label && "All relational have labels");
            LabelID patch_taddress = k_no_label;
            if (q.label < next_instruction_label_)
            {
                DEBUG_SMART_ASSERT(q.label < target_addresses_.size());
                patch_taddress = target_addresses_[q.label];
            }
            else
            {
                #warning "DOING NOTHING"
                // add_incomplete_jump(next_instruction_label(), );
            }
            return new vm::LabelArgument{patch_taddress};
        }(),
        extract_operant_by_trait<ir_opcode, IIT::arg1>(q.arg1),
        extract_operant_by_trait<ir_opcode, IIT::arg2>(q.arg2),
        q.loc
    );
    target_addresses_.push_back(reserve_next_label());
}

void
BytecodeGenerator::generate_getretval(const ir::Quad& q)
{
    DEBUG_SMART_ASSERT(q.opcode == ir::Opcode::GETRETVAL);
    generate<ir::Opcode::GETRETVAL, vm::Opcode::ASSIGN>(q);
    DEBUG_SMART_ASSERT(!result_.code.empty() && !result_.code.back().arg1);
    result_.code.back().arg1 = new vm::RetvalArgument{};
}

void
BytecodeGenerator::generate_funcstart(const ir::Quad& quad)
{
    DEBUG_SMART_ASSERT(quad.opcode == ir::Opcode::FUNCSTART);

    namespace IIT = ir::info_traits;
    static_assert(IIT::arg1(ir::Opcode::FUNCSTART) == IIT::Requirement::REQUIRED);

    const auto* const fn_expr = DEBUG_REQUIRE_PTR(static_cast<const ProgFuncExpr *>(quad.arg1));
    const auto* const fn_sym = DEBUG_REQUIRE_PTR(fn_expr->progfunc_symbol);
    DEBUG_SMART_ASSERT(fn_sym->stackframe_slot_count.is_assigned());
    result_.userfuncs.emplace_back(fn_sym->name, fn_sym->address, fn_sym->stackframe_slot_count);

    pending_returns_.emplace();
    generate<ir::Opcode::FUNCSTART, vm::Opcode::ENTERFUNC>(quad);
}

void
BytecodeGenerator::generate_return(const ir::Quad& quad)
{
    DEBUG_SMART_ASSERT(quad.opcode == ir::Opcode::RETURN);

    // We generate the 1st instruction `ASSIGN`:
    generate<ir::Opcode::RETURN, vm::Opcode::ASSIGN>(quad);
    DEBUG_SMART_ASSERT(!result_.code.empty() && !result_.code.back().result);
    result_.code.back().result = new vm::RetvalArgument{};

    DEBUG_SMART_ASSERT(!pending_returns_.empty() && "We are in RETURN, thus there is a function");

    // We generate the 2nd instruction `JUMP`:
    pending_returns_.top().push_back(reserve_next_label());

    static_assert(false, "I THINK I dont need the extra variable flag... Size() of result_.code "
                         "var is ENOUGH! If unnecessary... this creates synchronization bugs");
    result_.code.emplace_back(
        vm::Opcode::JUMP,
        nullptr,
        nullptr,
        new vm::LabelArgument{k_no_label},
        quad.loc
    );
}

void
BytecodeGenerator::generate_fundend(const ir::Quad& quad)
{
    DEBUG_SMART_ASSERT(quad.opcode == ir::Opcode::FUNCEND);
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
        case ir::Opcode::UMINUS:
            DEBUG_SMART_ASSERT(false && "NIY");
            break;
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
            generate_return(quad);
            break;
        case ir::Opcode::FUNCSTART:
            generate_funcstart(quad);
            break;
        case ir::Opcode::FUNCEND:
            generate_fundend(quad);
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
