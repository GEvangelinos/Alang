#include "vm/machine.hpp"
#include "vm/vm_memory.hpp"
#include "support/misc_tools.hpp"
#include <cmath>

namespace
{
namespace ALU
{
    constexpr auto k_arith_type_count = 2;
} // namespace ALU

namespace ALU::Arithmetic
{
    class DivisionByZero final {};

    using namespace alpha;

    constexpr auto k_arith_op_count = 5;

    using ArithmeticHandler = void (*)(
        vm::Memcell& lv,
        const vm::Memcell& rv1,
        const vm::Memcell& rv2
    );

    template <typename ArithOp, typename DivisorNumType>
        requires std::is_same_v<DivisorNumType, AlphaInt> ||
                 std::is_same_v<DivisorNumType, AlphaFloat>
    void manage_div_by_0(const DivisorNumType divisor)
    {
        constexpr bool is_division =
            std::is_same_v<ArithOp, std::divides<>> ||
            std::is_same_v<ArithOp, std::modulus<>>;
        if constexpr (is_division)
            if (divisor == 0)
                throw ALU::Arithmetic::DivisionByZero{};
    }

    template <typename ArithOp>
    void calc_int(vm::Memcell& lv, const AlphaInt rv1_value, const AlphaInt rv2_value)
    {
        manage_div_by_0<ArithOp>(rv2_value);
        lv.type = vm::Memcell::Type::INT;
        lv.data.int_value = ArithOp{}(rv1_value, rv2_value);
    }

    template <typename ArithOp>
    void calc_float(vm::Memcell& lv, const AlphaFloat rv1_value, const AlphaFloat rv2_value)
    {
        manage_div_by_0<ArithOp>(rv2_value);
        lv.type = vm::Memcell::Type::FLOAT;
        if constexpr (std::is_same_v<ArithOp, std::modulus<>>)
            lv.data.float_value = std::fmod(rv1_value, rv2_value);
        else
            lv.data.float_value = ArithOp{}(rv1_value, rv2_value);
    }

    template <typename ArithOp>
    void arith_ii(vm::Memcell& lv, const vm::Memcell& rv1, const vm::Memcell& rv2)
    {
        calc_int<ArithOp>(lv, rv1.data.int_value, rv2.data.int_value);
    }

    template <typename ArithOp>
    void arith_if(vm::Memcell& lv, const vm::Memcell& rv1, const vm::Memcell& rv2)
    {
        calc_float<ArithOp>(lv, static_cast<AlphaFloat>(rv1.data.int_value), rv2.data.float_value);
    }

    template <typename ArithOp>
    void arith_fi(vm::Memcell& lv, const vm::Memcell& rv1, const vm::Memcell& rv2)
    {
        calc_float<ArithOp>(lv, rv1.data.float_value, static_cast<AlphaFloat>(rv2.data.int_value));
    }

    template <typename ArithOp>
    void arith_ff(vm::Memcell& lv, const vm::Memcell& rv1, const vm::Memcell& rv2)
    {
        calc_float<ArithOp>(lv, rv1.data.float_value, rv2.data.float_value);
    }

    using HandlerMatrix2D = std::array<
        std::array<ArithmeticHandler, ALU::k_arith_type_count>,
        ALU::k_arith_type_count
    >;
    using HandlerMatrix3D = std::array<HandlerMatrix2D, ALU::Arithmetic::k_arith_op_count>;

    template <typename Operator>
    consteval HandlerMatrix2D generate_2d_handler()
    {
        return {
            {                                               /* [Arithmetic Opcode] */
                {{arith_ii<Operator>, arith_if<Operator>}}, /* rv1 INT   -> [rv2  INT, rv2 FLOAT] */
                {{arith_fi<Operator>, arith_ff<Operator>}}, /* rv1 FLOAT -> [rv2 INT, rv2 FLOAT] */
            }
        };
    }

    constexpr HandlerMatrix3D arith_dispatch_table = {
        generate_2d_handler<std::plus<>>(),
        generate_2d_handler<std::minus<>>(),
        generate_2d_handler<std::multiplies<>>(),
        generate_2d_handler<std::divides<>>(),
        generate_2d_handler<std::modulus<>>(),
    };
} // namespace ALU::Arithmetic

namespace ALU::Relational
{
    using namespace alpha;

    constexpr auto k_rel_op_count = 4;

    using RelationalHandler = bool(*)(
        const vm::Memcell& rv1,
        const vm::Memcell& rv2
    );

    template <typename ArithOp, typename RvalueNumType>
        requires std::is_same_v<RvalueNumType, AlphaInt> ||
                 std::is_same_v<RvalueNumType, AlphaFloat>
    bool calc_rel(const RvalueNumType rv1_value, const RvalueNumType rv2_value)
    {
        const bool result = ArithOp{}(rv1_value, rv2_value);
        return result;
    }

    template <typename ArithOp>
    bool rel_ii(const vm::Memcell& rv1, const vm::Memcell& rv2)
    {
        return calc_rel<ArithOp>(rv1.data.int_value, rv2.data.int_value);
    }

    template <typename ArithOp>
    bool rel_if(const vm::Memcell& rv1, const vm::Memcell& rv2)
    {
        return calc_rel<ArithOp>(static_cast<AlphaFloat>(rv1.data.int_value), rv2.data.float_value);
    }

    template <typename ArithOp>
    bool rel_fi(const vm::Memcell& rv1, const vm::Memcell& rv2)
    {
        return calc_rel<ArithOp>(rv1.data.float_value, static_cast<AlphaFloat>(rv2.data.int_value));
    }

    template <typename ArithOp>
    bool rel_ff(const vm::Memcell& rv1, const vm::Memcell& rv2)
    {
        return calc_rel<ArithOp>(rv1.data.float_value, rv2.data.float_value);
    }

    using HandlerMatrix2D = std::array<
        std::array<RelationalHandler, ALU::k_arith_type_count>,
        ALU::k_arith_type_count
    >;
    using HandlerMatrix3D = std::array<HandlerMatrix2D, ALU::Relational::k_rel_op_count>;

    template <typename Operator>
    consteval HandlerMatrix2D generate_2d_handler()
    {
        return {
            {                                           /* [Arithmetic Opcode] */
                {{rel_ii<Operator>, rel_if<Operator>}}, /* rv1 INT   -> [rv2  INT, rv2 FLOAT] */
                {{rel_fi<Operator>, rel_ff<Operator>}}, /* rv1 FLOAT -> [rv2 INT, rv2 FLOAT] */
            }
        };
    }

    constexpr HandlerMatrix3D rel_dispatch_table = {
        generate_2d_handler<std::less<>>(),
        generate_2d_handler<std::less_equal<>>(),
        generate_2d_handler<std::greater<>>(),
        generate_2d_handler<std::greater_equal<>>(),
    };
} // namespace ALU::Relational
} // namespace

namespace alpha::vm
{
template <Opcode first, Opcode last>
std::optional<Machine::DecodedOperands>
Machine::decode_arithmetic_operands(const vm::Instruction& inst)
{
    const auto inst_opcode = inst.opcode;
    DMASSERT(inst_opcode >= first, inst_opcode <= last);

    const vm::Memcell& rv1 = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg1.get(), &reg_a_));
    const vm::Memcell& rv2 = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg2.get(), &reg_b_));

    if (!(rv1.is_arithmetic() && rv2.is_arithmetic())) [[unlikely]]
    {
        error("not a number in arithmetic Operation");
        return std::nullopt;
    }

    const auto resolve = [](const Memcell::Type mt) noexcept
    {
        using MemcellUT = std::underlying_type_t<Memcell::Type>;
        static_assert(
            static_cast<MemcellUT>(Memcell::Type::INT) + 1 ==
            static_cast<MemcellUT>(Memcell::Type::FLOAT),
            "Memcell arithmetic types must be contiguous for array indexing"
        );
        const auto result = static_cast<MemcellUT>(mt) - static_cast<MemcellUT>(Memcell::Type::INT);
        DMASSERT(result >= 0, result <= std::numeric_limits<u8>::max());
        return result;
    };

    using OpcodeUT = std::underlying_type_t<vm::Opcode>;
    const auto op_idx = static_cast<OpcodeUT>(inst_opcode) - static_cast<OpcodeUT>(first);
    DMASSERT(op_idx >= 0, op_idx <= std::numeric_limits<OpcodeUT>::max());
    const u8 t1_idx = resolve(rv1.type);
    const u8 t2_idx = resolve(rv2.type);

    return DecodedOperands{
        .rv1 = rv1,
        .rv2 = rv2,
        .op_idx = static_cast<OpcodeUT>(op_idx),
        .t1_idx = t1_idx,
        .t2_idx = t2_idx,
    };
}

[[nodiscard]] bool
validate_instruction(const vm::Instruction* const inst) {}

void
Machine::execute_arithmetic(const vm::Instruction& inst)
{
    const std::optional<DecodedOperands> operands =
        decode_arithmetic_operands<Opcode::ADD, Opcode::MOD>(inst);
    if (!operands) [[unlikely]]
        return;
    const auto handler =
        ALU::Arithmetic::arith_dispatch_table[operands->op_idx][operands->t1_idx][operands->t2_idx];

    vm::Memcell& lv = *DEBUG_REQUIRE_PTR(translate_operand(inst.result.get(), nullptr));
    lv.clear();
    try { handler(lv, operands->rv1, operands->rv2); }
    catch (const ALU::Arithmetic::DivisionByZero&) { error("Division by 0"); }
}

void
Machine::execute_relational_branch(const vm::Instruction& inst)
{
    const std::optional<DecodedOperands> operands =
        decode_arithmetic_operands<Opcode::JLT, Opcode::JGE>(inst);
    if (!operands)
        return;
    const auto handler = ALU::Relational::rel_dispatch_table
        [operands->op_idx][operands->t1_idx][operands->t2_idx];

    const bool should_take_branch = handler(operands->rv1, operands->rv2);
    if (should_take_branch)
    {
        DMASSERT(!!inst.result);
        DMASSERT(inst.result->type == Argument::Type::LABEL);
        const CodeAddress branch_target =
            static_cast<const LabelArgument*>(inst.result.get())->value;
        pc_ = branch_target.value - 1; // @PC_TAG@ -1 is required, as addresses start from 1;
    }
}

void
Machine::execute_equality_branch(const vm::Instruction& inst)
{
    DMASSERT(!!inst.result);

    const vm::Memcell& rv1 = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg1.get(), &reg_a_));
    const vm::Memcell& rv2 = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg2.get(), &reg_b_));

    bool condition = false;
    if (rv1.type == Memcell::Type::UNDEF || rv2.type == Memcell::Type::UNDEF)
    {
        error("Undef involved in equality operator.");
        return;
    }
    if (rv1.type == Memcell::Type::BOOL || rv2.type == Memcell::Type::BOOL)
        condition = rv1.to_bool() == rv2.to_bool();
    else if (rv1.type == Memcell::Type::NIL || rv2.type == Memcell::Type::NIL)
        condition = rv1.type == Memcell::Type::NIL && rv2.type == Memcell::Type::NIL;
    else if (rv1.type != rv2.type)
    {
        error(FMT::format("{} == {} is illegal", to_string(rv1.type), to_string(rv2.type)));
        return;
    }
    else
    {
        DMASSERT(rv1.type == rv2.type);
        switch (rv1.type)
        {
        case Memcell::Type::UNDEF:
        case Memcell::Type::NIL:
        case Memcell::Type::BOOL: DMASSERT(false && "Should be already caught above");
        case Memcell::Type::INT:
            condition = rv1.data.int_value == rv2.data.int_value;
            break;
        case Memcell::Type::FLOAT:
            condition = rv1.data.float_value == rv2.data.float_value;
            break;
        case Memcell::Type::STRING:
            condition = std::strcmp(rv1.data.str_value, rv2.data.str_value) == 0;
            break;
        case Memcell::Type::TABLE:
            condition = rv1.data.table_value == rv2.data.table_value; // We compare ADDRESS SIZE
            break;
        case Memcell::Type::PROGFUNC:
            condition = rv1.data.progfunc_index == rv2.data.progfunc_index;
            break;
        case Memcell::Type::LIBFUNC:
            condition = rv1.data.libfunc_id == rv2.data.libfunc_id;
            break;
        default: DMASSERT(false && "Unknown vm::Memcell::Type");
        }
    }


    DMASSERT(inst.result->type == Argument::Type::LABEL);
    if (condition)
        pc_ = static_cast<const LabelArgument*>(inst.result.get())->value.value -1;
     // @PC_TAG@ - 1 is required, as addresses start from 1
}
} // namespace alpha::vm
