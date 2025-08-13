// FILE: assert invariants on generated code (.cpp)
// Purpose: Compile-time invariants that validate the generated IR opcode metadata.
//
// This TU relies on generated traits and the complete list of opcodes.
// Each invariant is a consteval function asserted via static_assert so that
// violations produce clear errors at compile time.

#include <array>

#include <parser/ir_opcode.gen.hpp>
#include <parser/ir_opcode_info_traits.gen.hpp>
#include <parser/ir_opcode_opt_traits.gen.hpp>

namespace
{
using alpha::ir::Opcode;
using namespace alpha::ir::info_traits;
using namespace alpha::ir::opt_traits;

// -----------------------------------------------------------------------------
// Positive-sense wrappers (the generated predicates are negated).
// -----------------------------------------------------------------------------
[[nodiscard]] consteval bool is_emittable(const Opcode opc)
{
    return !alpha::ir::info_traits::is_non_emittable(opc);
}

[[nodiscard]] consteval bool is_executable(const Opcode opc)
{
    return !alpha::ir::info_traits::is_non_executable(opc);
}

// Invariant: Executable -> Emittable
// Rationale: Anything the evaluator/VM can execute must be eligible for code
// emission. If an opcode is marked executable while also non-emittable, traits
// are inconsistent.
[[nodiscard]] consteval bool inv0_executable_implies_emittable()
{
    for (const auto opc: alpha::ir::all_opcodes_array)
        if (is_executable(opc) && !is_emittable(opc))
            return false;
    return true;
}

// Invariant: Branching opcodes produce no result.
// Rationale: Branching changes control-flow; it should not define a value in a
// result operand slot.
[[nodiscard]] consteval bool inv1_branching_has_no_result()
{
    for (const auto opc: alpha::ir::all_opcodes_array)
        if (is_branching(opc) && result(opc) != Requirement::NONE)
            return false;
    return true;
}

// Invariant: Non-emittable opcodes have no operands.
// Rationale: Non-emittable items are metadata/sentinels and must not carry
// expression structure (no result, no args).
[[nodiscard]] consteval bool inv2_non_emittable_has_no_expr()
{
    const auto has_any_operand = [](const Opcode opc)
    {
        return result(opc) != Requirement::NONE ||
               arg1(opc) != Requirement::NONE ||
               arg2(opc) != Requirement::NONE;
    };

    for (const auto opc: alpha::ir::all_opcodes_array)
    {
        if (is_non_emittable(opc) && has_any_operand(opc)) { return false; }
    }
    return true;
}

// Invariant: Emittable & foldable -> Arg1 is REQUIRED.
// Rationale: Constant folding operates on a primary operand; ensuring arg1 is
// present simplifies folder and codegen assumptions.
[[nodiscard]] consteval bool inv3_emittable_and_foldable_requires_arg1()
{
    for (const auto opc: alpha::ir::all_opcodes_array)
    {
        if (is_emittable(opc) && is_foldable(opc) &&
            arg1(opc) != Requirement::REQUIRED) { return false; }
    }
    return true;
}

// Invariant: If Arg1 is NONE, Arg2 must also be NONE.
// Rationale: Positional operand discipline -- you can’t have arg2 without arg1.
[[nodiscard]] consteval bool inv4_no_arg1_implies_no_arg2()
{
    for (const auto opc: alpha::ir::all_opcodes_array)
        if (arg1(opc) == Requirement::NONE && arg2(opc) != Requirement::NONE)
            return false;
    return true;
}

// Invariant: Non-emittable ⇒ Non-executable
// Rationale: If something cannot be emitted into code, it must not be marked
// executable either; otherwise traits disagree about its lifecycle.
[[nodiscard]] consteval bool inv5_non_emittable_implies_non_executable()
{
    for (const auto opc: alpha::ir::all_opcodes_array)
        if (is_non_emittable(opc) && is_executable(opc))
            return false;
    return true;
}

// Invariant: Emittable with exactly 1 optional operand ⇒ Arg1 present.
// Rationale: If an instruction is emittable and advertises exactly one optional
// operand, that operand must be represented in arg1.
[[nodiscard]] consteval bool inv6_emittable_with_one_opt_operand_requires_arg1()
{
    for (const auto opc: alpha::ir::all_opcodes_array)
        if (is_emittable(opc) &&
            opt_operands(opc) == 1 &&
            arg1(opc) == Requirement::NONE)
            return false;

    return true;
}

// Invariant 7: Emittable with more than 1 optional operand ⇒ Arg1 present.
// Rationale: If an instruction is emittable and has multiple optional operands,
// Arg1 must still exist to maintain positional operand ordering.
[[nodiscard]] consteval bool inv7_emittable_with_multi_opt_operands_requires_arg1()
{
    for (const auto opc: alpha::ir::all_opcodes_array)
        if (is_emittable(opc) &&
            opt_operands(opc) > 1 &&
            arg1(opc) == Requirement::NONE)
            return false;
    return true;
}

// clang-format off
static_assert(inv0_executable_implies_emittable(),                    "Invariant failed: Executable -> Emittable.");
static_assert(inv1_branching_has_no_result(),                         "Invariant failed: Branching opcodes must not produce a result.");
static_assert(inv2_non_emittable_has_no_expr(),                       "Invariant failed: Non-emittable opcodes must have no expr to emit.");
static_assert(inv3_emittable_and_foldable_requires_arg1(),            "Invariant failed: Emittable & foldable -> arg1 is REQUIRED.");
static_assert(inv4_no_arg1_implies_no_arg2(),                         "Invariant failed: Arg2 must be NONE while Arg1 is NONE.");
static_assert(inv5_non_emittable_implies_non_executable(),            "Invariant failed: Non-emittable -> Non-executable.");
static_assert(inv6_emittable_with_one_opt_operand_requires_arg1(),    "Invariant failed: Emittable + 1 optional operand -> Arg1 present.");
static_assert(inv7_emittable_with_multi_opt_operands_requires_arg1(), "Invariant failed: Emittable + >1 optional operands -> Arg1 present.");
// clang-format on
} // namespace
