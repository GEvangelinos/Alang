#include "bytecode/abc_generator.hpp"
#include <assert.h>

#include "parser/ir_opcode_info_traits.gen.hpp"
#include "core/ir/ir_expr.hpp"
#include "support/dependent_false.hpp"

namespace alpha
{
vm::Program::StringID
ABC_Generator::intern_string_literal(const ConstStringExpr& string_expr)
{
    const auto it = result_.str_literal_table.try_emplace(
        string_expr.value, result_.str_literal_table.size()
    ).first;
    result_.metadata.total_string_size += string_expr.value.size;
    const auto str_id = it->second;
    return str_id;
}

vm::Program::LibfuncID
ABC_Generator::intern_libfunc_name(const LibFuncExpr& libfunc_expr)
{
    const auto it = result_.libfunc_name_table.try_emplace(
        libfunc_expr.libfunc_symbol->name, result_.libfunc_name_table.size()
    ).first;
    const auto libname_id = it->second;
    return libname_id;
}


const vm::Argument*
ABC_Generator::make_argument(const Expr& expr)
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
        DMASSERT(expr.has_var_symbol());
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
ABC_Generator::extract_operant_by_requirement_trait(const Expr* const e)
{
    namespace IIT = ir::info_traits;
    constexpr IIT::Requirement req = trait_func(ir_quad_opcode);
    if constexpr (req == IIT::Requirement::REQUIRED)
        return make_argument(*DEBUG_REQUIRE_PTR(e));
    else if constexpr (req == IIT::Requirement::NONE)
        return nullptr;
    else if constexpr (req == IIT::Requirement::OPTIONAL)
        return e ? make_argument(*e) : nullptr;
    else static_assert(always_false_v<decltype(ir_quad_opcode)>, "Unknown Requirement value");
}

template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
void
ABC_Generator::generate(const ir::Quad& q)
{
    namespace IIT = ir::info_traits;
    DMASSERT(ir_opcode == q.opcode);
    target_addresses_.push_back(next_instruction_label());
    result_.instructions.emplace_back(
        vm_opcode,
        extract_operant_by_requirement_trait<ir_opcode, IIT::result>(q.result),
        extract_operant_by_requirement_trait<ir_opcode, IIT::arg1>(q.arg1),
        extract_operant_by_requirement_trait<ir_opcode, IIT::arg2>(
            ir_opcode == ir::Opcode::UMINUS ? &k_static_int_neg1_expr : q.arg2),
        q.loc
    );
}

template <ir::Opcode ir_opcode, vm::Opcode vm_opcode>
void
ABC_Generator::generate_relational(const ir::Quad& q)
{
    namespace IIT = ir::info_traits;
    DMASSERT(ir_opcode == q.opcode);
    target_addresses_.push_back(next_instruction_label());
    result_.instructions.emplace_back(
        vm_opcode,
        new vm::LabelArgument{q.label},
        extract_operant_by_requirement_trait<ir_opcode, IIT::arg1>(q.arg1),
        extract_operant_by_requirement_trait<ir_opcode, IIT::arg2>(q.arg2),
        q.loc
    );
}

void
ABC_Generator::generate_uminus(const ir::Quad& q)
{
    namespace IIT = ir::info_traits;
    DMASSERT(ir::Opcode::UMINUS == q.opcode);
    target_addresses_.push_back(next_instruction_label());
    result_.instructions.emplace_back(
        vm::Opcode::MUL,
        extract_operant_by_requirement_trait<ir::Opcode::MUL, IIT::result>(q.result),
        extract_operant_by_requirement_trait<ir::Opcode::MUL, IIT::arg1>(q.arg1),
        extract_operant_by_requirement_trait<ir::Opcode::MUL, IIT::arg2>(&k_static_int_neg1_expr),
        q.loc
    );
}

void
ABC_Generator::generate_getretval(const ir::Quad& q)
{
    DMASSERT(q.opcode == ir::Opcode::GETRETVAL);
    generate<ir::Opcode::GETRETVAL, vm::Opcode::ASSIGN>(q);
    DMASSERT(!result_.instructions.empty() && !result_.instructions.back().arg1);
    result_.instructions.back().arg1 = new vm::RetvalArgument{};
}

void
ABC_Generator::generate_funcstart(const ir::Quad& quad)
{
    DMASSERT(quad.opcode == ir::Opcode::FUNCSTART);

    namespace IIT = ir::info_traits;
    static_assert(IIT::arg1(ir::Opcode::FUNCSTART) == IIT::Requirement::REQUIRED);

    const auto* const fn_expr = DEBUG_REQUIRE_PTR(static_cast<const ProgFuncExpr *>(quad.arg1));
    const auto* const fn_sym = DEBUG_REQUIRE_PTR(fn_expr->progfunc_symbol);
    DMASSERT(fn_sym->stackframe_slot_count.is_assigned());
    result_.progfuncs.emplace_back(fn_sym->name, fn_sym->address, fn_sym->stackframe_slot_count);
    generate<ir::Opcode::FUNCSTART, vm::Opcode::ENTERFUNC>(quad);
}

void
ABC_Generator::generate_return(const ir::Quad& quad)
{
    DMASSERT(quad.opcode == ir::Opcode::RETURN);

    // We generate the 1st instruction `ASSIGN`:
    generate<ir::Opcode::RETURN, vm::Opcode::ASSIGN>(quad);
    DMASSERT(!result_.instructions.empty() && !result_.instructions.back().result);
    result_.instructions.back().result = new vm::RetvalArgument{};
}

vm::Program
ABC_Generator::build_program(const std::vector<ir::Quad>& program_ir_quads) &&
{
    for (u64 i = 0; i < program_ir_quads.size(); ++i)
    {
        #define CASE_BASIC(ir_op, vm_op) case ir::Opcode::ir_op: generate<ir::Opcode::ir_op, vm::Opcode::vm_op>(quad); break
        #define CASE_RELATIONAL(ir_op, vm_op) case ir::Opcode::ir_op: generate_relational<ir::Opcode::ir_op, vm::Opcode::vm_op>(quad); break
        switch (auto& quad = program_ir_quads[i]; quad.opcode)
        {
        CASE_BASIC(ASSIGN, ASSIGN);
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
        case ir::Opcode::UMINUS: generate_uminus(quad);
            break;
        case ir::Opcode::GETRETVAL: generate_getretval(quad);
            break;
        case ir::Opcode::RETURN: generate_return(quad);
            break;
        case ir::Opcode::FUNCSTART: generate_funcstart(quad);
            break;
        CASE_BASIC(FUNCEND, EXITFUNC);
        case ir::Opcode::NOT:
        case ir::Opcode::AND:
        case ir::Opcode::OR: DMASSERT(false && "Should be unused ir opcodes");
        default: DMASSERT(false && "Unknown ir::Opcode");
        }
        #undef CASE_BASIC
        #undef CASE_RELATIONAL
        DMASSERT(
            i + 1 == result_.instructions.size() &&
            "Instruction pointer desync: 1:1 mapping between Quads and ABC instructions violated. "
            "Absolute jump offsets are now corrupted. Check the last generated opcode for "
            "unintended expansion (1:N) or omission (1:0)."
        );
    }
    return std::move(result_);
}
} // namespace alpha
